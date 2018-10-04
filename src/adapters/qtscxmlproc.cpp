// (C) Copyright IBM Corp. 2018.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "qtscxmlproc.hpp"
#include "moc_qtscxmlproc.hpp"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QJsonValue>
#include <QtCore/QLoggingCategory>
#include <QtCore/QStandardPaths>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>
#include <QtCore/QtGlobal>
#include <QtQml/QJSEngine>
#include <QtScxml/QScxmlEcmaScriptDataModel>
#include <QtScxml/private/qscxmlstatemachine_p.h>

#include <cassert>
#include <codecvt>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>

using namespace scxml;

// --------------------------------------------------------------------------------
// ctor/dtor
// --------------------------------------------------------------------------------

qtscxmlproc::qtscxmlproc (void) :
    scxmlproc (),
    _application (nullptr),
    _machine (nullptr),
    _engine (nullptr),
    _monitor (nullptr)
{
}

qtscxmlproc::qtscxmlproc (QCoreApplication* app) :
    scxmlproc (),
    _application (app),
    _machine (nullptr),
    _engine (nullptr),
    _monitor (nullptr)
{
}

qtscxmlproc::qtscxmlproc (const qtscxmlproc& p) :
    scxmlproc ()
{
    assert (false);
}

qtscxmlproc::~qtscxmlproc (void)
{
    if (_machine) delete _machine;
}

void
qtscxmlproc::load (const std::string& scxml_url)
{
    //_interpreter = uscxml::Interpreter::fromURL (scxml_url);
    //Factory* factory = Factory::getInstance ();
    //assert (factory->hasDataModel ("ecmascript"));

    _machine = QScxmlStateMachine::fromFile (QString (scxml_url.c_str ()));
    assert (_machine);
    if (!_machine->parseErrors ().isEmpty ())
    {
        QTextStream errs (stderr, QIODevice::WriteOnly);
        const auto errors = _machine->parseErrors ();
        for (const QScxmlError &error : errors) qCritical () << error.toString ();
        throw std::runtime_error ("Parse_error");
    }

    qInfo () << ";; statechart:" << _machine->name ();
    QStringList names = _machine->stateNames ();
    for (int i = 0; i < names.size (); i++)
        //qInfo () << ";; state: " << names.at (i).toLocal8Bit().constData () << std::endl;
        qInfo () << ";; state:" << names.at (i);

    // EcmaScript
    QScxmlDataModel* datamodel = _machine->dataModel ();
    assert (datamodel);
    //_machine->setDataModel (new QScxmlEcmaScriptDataModel ());


    assert (_application);
    _machine->setParent (_application);
}

// --------------------------------------------------------------------------------
// run
// --------------------------------------------------------------------------------

void
qtscxmlproc::run (void)
{
    if (!_machine->isInitialized ()) _machine->init ();
    if (!_machine->isInitialized ())
    {
        qCritical () << "** ERROR: statemachine not initialized";
        _application-> quit ();
        throw std::runtime_error ("Failure");
    }
    assert (_machine->isInitialized ());
    qDebug () << ";; start running";

    _machine->start ();
    assert (_machine->isRunning ());

    assert (_application);
    _application->exec ();

}

// --------------------------------------------------------------------------------
// step
// --------------------------------------------------------------------------------

void
qtscxmlproc::step (void)
{
    qDebug () << ";; step";
    assert (_eventin);
    //assert (_machine->isRunning ());

    //std::clog << _interpreter.serialize () << std::endl;
    //LOGD (USCXML_DEBUG) << interpreter.getImpl ().get ()->serialize () << std::endl;
    //if (_traceout) _datamodel_print (*_traceout);

    QScxmlStateMachinePrivate* smp = ((_QScxmlStateMachine*) _machine)->d ();
    if (smp->m_isProcessingEvents) return;
    // ** note:
    //    when m_isProcessingEvents is true,
    //    queueProcessEvents, called from submitEvent(e), returns without invoking doProcessEvents.
    //    then the event, e, could remain in the queue without being processed.

    QScxmlEvent* e = new QScxmlEvent ();
    try { event_read (*e); }
    catch (std::runtime_error ex)
    {
        if (strcmp (ex.what (), "End_of_file") == 0)
        {
            qDebug () << ";; EOF reached";
            if (_machine->isRunning ()) return;

            _application->quit ();
            //throw std::runtime_error ("Exit");
            qDebug () << ";; quit on EOF";
            return;
        }
        else if (strcmp (ex.what (), "Not_found") == 0)
            // no event found
            // there's still a chance that there will be.
        {
            //usleep (100000);	// 100ms
            // ** this sometimes causes ignoring submitted events.. why?
            return;
        }
        else
            throw ex;
    }
    //std::clog << e << std::endl;
    //_interpreter.receive (e);
    qDebug () << ";; submitEvent:" << e->name () << e->eventType ();
    _machine->submitEvent (e);
      // submitEvent (e) -> routeEvent (e) -> postEvent (e)
      // -> m_internalQueue.enqueue (e); m_eventLoopHook.queueProcessEvents
    {
        nlohmann::json obj =
            {{"submitEvent", {{"name", e->name ().toStdString ()},
                              {"data", e->data ().toString ().toStdString ()},
                              {"type", e->eventType ()}}}};
        _traceout->write (obj);
    }

    //assert (_machine->isRunning ());
    // ** could be paused

    return;

    /*
    uscxml::InterpreterState state = _interpreter.getState ();
    switch (state)
    {
    case uscxml::USCXML_FINISHED:
        return (state);    

    case uscxml::USCXML_MACROSTEPPED:
        // A macrostep is a series of one or more microsteps ending in a configuration where the internal event queue is empty and no transitions are enabled by NULL.
        {
            //std::clog << _interpreter.serialize () << std::endl;
            //LOGD (USCXML_DEBUG) << interpreter.getImpl ().get ()->serialize () << std::endl;
            if (_traceout) _datamodel_print (*_traceout);

            uscxml::Event e;
            try { event_read (e); }
            catch (std::runtime_error ex)
            {
                if (strcmp (ex.what (), "End_of_file") == 0)
                    // events still may arrive later (delayed send)
                    assert (true);
                else
                    throw ex;
            }
            //std::clog << e << std::endl;
            _interpreter.receive (e);

            break;
        }
    default:
        {}
    }

    state = _interpreter.step ();
    const char* state_names[] = {"UNDEF", "IDLE", "INITIALIZED", "INSTANTIATED", "MICROSTEPPED", "MACROSTEPPED", "CANCELLED"};
    const char* state_name = state == uscxml::USCXML_FINISHED ? "FINISHED" : state_names[state];
    std::clog << ";; InterpreterState: " << state_name << std::endl;

    switch (state)
    {
    case uscxml::USCXML_UNDEF:
        throw std::runtime_error ("Not_found");
    case uscxml::USCXML_IDLE:
        return (step ());
    default:
        {}
    }

    return (state);
    */
}

// --------------------------------------------------------------------------------
// event in/out
// --------------------------------------------------------------------------------

void
qtscxmlproc::event_read (QScxmlEvent& e)
{
    nlohmann::json obj;
    _eventin->read (obj);

    if (obj.is_null ()) throw std::runtime_error ("Not_found");
    if (obj.find ("event") != obj.end ()) obj = obj["event"];

    qDebug () << ";; event_read:" << obj.dump ().c_str ();

    if (!obj["name"].is_string ()) throw std::runtime_error ("Invalid_argument");
    assert (obj["name"].is_string ());
    std::string name = obj["name"];
    e.setName (QString (name.c_str ()));
    e.setEventType (QScxmlEvent::InternalEvent);	// raise
    //e.setEventType (QScxmlEvent::ExternalEvent);	// send

    auto data = obj["data"];
    const char* str = data.dump ().c_str ();
    //const char* str = data.get<std::string> ().c_str ();
    QJsonValue val (str);
    e.setData (val.toVariant ());

    qDebug () << ";; event_read:"
              << "name:" << name.c_str ()
              << "data:" << str;
}

void
qtscxmlproc::event_write (jsonostream& s, const QScxmlEvent& e)
{
    if (e.eventType () != QScxmlEvent::ExternalEvent) return;

    nlohmann::json obj = {{"event", {{"name", e.name ().toStdString ()}}}};
    s.write (obj);
}

// --------------------------------------------------------------------------------
// traceout
// --------------------------------------------------------------------------------

// --------------------------------------------------------------------------------
// monitor
// --------------------------------------------------------------------------------

/*
class monitor : public QObject
{
Q_OBJECT

public:
    //monitor (void) : QObject () {}
    //explicit monitor (monitor&) : QObject () {}
    //explicit monitor (QObject* parent = 0) : QObject (parent) {}
    //monitor (QObject* parent = 0) : QObject (parent) {}
    //explicit monitor (QObject* parent = 0);
    monitor ();
    virtual ~monitor ();

    void on_entry (bool active)
    {
    }

    void on_exit (bool active)
    {
    }

signals:

public slots:
    void log (const QString &label, const QString &msg)
    //void log ()
    {
        std::cerr << "hello";
    }

};
*/

//monitor::monitor (void) {}
//monitor::~monitor () {}

void
monitor::state_cb (const QString& name, bool active)
{
    qDebug () << ";; state_cb:" << name
              << (active ? "(on entry)" : "(on exit)");

    if (!active) return;

    //std::ostringstream out;
    //out << "{\"state\" : {";
    //print_attributes (out, state);
    //out << "}}\n";
    //nlohmann::json obj = nlohmann::json::parse (out.str ());
    nlohmann::json obj = {{"state", {{"id", name.toStdString ()}}}};

    assert (_traceout);
    _traceout->write (obj);
}

/*
void
monitor::log (const QString &label, const QString &msg)
{
    qDebug () << "[log]" << label << " : " << msg;
}
*/

// --------------------------------------------------------------------------------
// setup
// --------------------------------------------------------------------------------

void
qtscxmlproc::verbosity_set (int v)
{
    _verbosity = v;

    QLoggingCategory* default_cat = QLoggingCategory::defaultCategory ();
    //qDebug () << QStandardPaths::standardLocations (QStandardPaths::GenericConfigLocation);
    switch (_verbosity)
    {
    case -1:
        QLoggingCategory::setFilterRules ("*.debug=false\n"
                                          "*.info=false\n"
                                          "*.warning=false\n"
                                          "*.critical=false\n");
        break;
    case 0:
        QLoggingCategory::setFilterRules ("default.debug=false\n"
                                          "default.info=false\n"
                                          "default.warning=false\n"
                                          "scxml.statemachine.debug=false\n"
                                          "qt.scxml.statemachine.debug=false\n");
        assert (!default_cat->isDebugEnabled ());
        assert (!default_cat->isInfoEnabled ());
        assert (!default_cat->isWarningEnabled ());
        assert (default_cat->isCriticalEnabled ());
        break;
    case 1:
        QLoggingCategory::setFilterRules ("qt.scxml.statemachine.debug=false\n");
        break;
    case 2:
        QLoggingCategory::setFilterRules ("*.debug=true\n");
        //QLoggingCategory* cat = (QLoggingCategory*) &qscxmlLog ();
        //cat->setEnabled (QtDebugMsg, true);
        //assert (cat->isDebugEnabled ());
        break;
    default:
        {}
    }

}

void
qtscxmlproc::setup (void)
{
    assert (_machine);

    if (!_monitor) _monitor = new monitor ();
    //_monitor->setParent (_application);
    _monitor->_traceout = _traceout;

    // state callback -- invoked when entering/exiting a state
    //std::function<void (bool)> on_entry = QScxmlStateMachine::onEntry
    //mon->onEntry (obj, meth);
    //_machine->connectToState ("hello", _monitor, "on_entry");
    //_machine->connectToState ("hello", _monitor, &monitor::on_entry);
    QStringList states = _machine->stateNames ();
    for (int i = 0; i < states.size (); i++)
    {
        QString state = states.at (i);
        _machine->connectToState (state, _monitor,
                                  [this, state](bool active) {
                                      _monitor->state_cb (state, active);
                                  });
    }

    // ScxmlEventRouter SIGNAL: eventOccurred (QScxmlEvent)
    // ** note: only those events of the ExternalEvent type are captured
    _machine->connectToEvent ("*",
                              [this](const QScxmlEvent& e) {
                                  qDebug () << ";; event:" << e.name () << e.eventType ();
                                  event_write (*_eventout, e);
                              });

    // SIGNAL: dataModelChanged (QScxmlDataModel* datamodel)
    QObject::connect (_machine, &QScxmlStateMachine::dataModelChanged,
                      [this](QScxmlDataModel* datamodel) {
                          qDebug () << ";; SIGNAL: dataModelChanged";
                      });

    // SIGNAL: finished ()
    //QObject::connect (_machine, SIGNAL (finished ()), _application, SLOT (quit ()));
    //QObject::connect (_machine, &QScxmlStateMachine::finished,
    //                  _application, &QCoreApplication::quit);
    QObject::connect (_machine, &QScxmlStateMachine::finished,
                      [this]() {
                          qDebug () << ";; SIGNAL: finished";
                          _application->quit (); 
                      });

    // SIGNAL: initializedChanged (initialized)
    QObject::connect (_machine, &QScxmlStateMachine::initializedChanged,
                      [this](bool initialized) {
                          qDebug () << ";; SIGNAL: initializedChanged" << initialized;
                      });

    // SIGNAL: log
    //QObject::connect (_machine, SIGNAL (log (const QString&, const QString&)),
    //                  _monitor, SLOT (log (const QString&, const QString&)));
    //QObject::connect (_machine, &QScxmlStateMachine::log, _monitor, &monitor::log);
    QObject::connect (_machine, &QScxmlStateMachine::log,
                      [](const QString& label, const QString& msg) {
                          qInfo () << "[log]" << label << " : " << msg;
                      });

    // SIGNAL: reachedStableState
    QObject::connect (_machine, &QScxmlStateMachine::reachedStableState,
                      [this]() {
                          qDebug () << ";; SIGNAL: reachedStableState";
                          step ();
                      });

    // SIGNAL: runningChanged (bool running)
    QObject::connect (_machine, &QScxmlStateMachine::runningChanged,
                      [this](bool running) {
                          qDebug () << ";; SIGNAL: runningChanged: " << running;
                      });

    // timer
    QTimer* timer = new QTimer (_application);
    QObject::connect (timer, &QTimer::timeout,
                      [this]() {
                          qDebug () << ";; SIGNAL: timeout";
                          //assert (_machine->isRunning ());
                          step ();
                      });
    timer->start ();

    // hack
    _hack ();
    assert (_engine);

    // init
    _machine->init ();
}

void
qtscxmlproc::version (void)
{
    std::cerr << "QtCore:\t"
              << QT_VERSION_STR
              << std::endl;

    std::cerr << "Mosquitto:\t"
              << LIBMOSQUITTO_MAJOR << "."
              << LIBMOSQUITTO_MINOR << "."
              << LIBMOSQUITTO_REVISION
              << std::endl;
}

// --------------------------------------------------------------------------------
// QJSEngine hack
// --------------------------------------------------------------------------------

void
JSConsole::log (const QString msg)
{
    std::string str = msg.toStdString ();
    qDebug () << "jsConsole: "<< str.c_str () << msg.size ();
}

void
qtscxmlproc::_hack (void)
{
    QScxmlEcmaScriptDataModel* datamodel = (QScxmlEcmaScriptDataModel*) _machine->dataModel ();
    _QScxmlEcmaScriptDataModel* _datamodel = (_QScxmlEcmaScriptDataModel*)datamodel;
    _engine = _datamodel->d ()->assertEngine ();
    //QJSEngine* engine = nullptr;
    assert (_engine);

    QJSValue global = _engine->globalObject ();
    assert (global.isObject ());

    // raiseEvent

    // sendEvent

    // _data
    if (!global.hasProperty ("_data"))
    {
        _engine->evaluate ("_data = {}");
    }
    assert (global.hasProperty ("_data"));

    // console
    _engine->installExtensions (QJSEngine::TranslationExtension | QJSEngine::ConsoleExtension);
    return;

    // console.log
    JSConsole* console = new JSConsole ();
    QJSValue console_val = _engine->newQObject (console);
    _engine->globalObject ().setProperty ("console", console_val);
}
