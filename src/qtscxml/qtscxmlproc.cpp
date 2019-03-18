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
#include "esprimitives.hpp"
#include "version.hpp"
#include "moc_qtscxmlproc.hpp"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
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
#include <unistd.h>

using namespace scxml;

// --------------------------------------------------------------------------------
// ctor/dtor
// --------------------------------------------------------------------------------

qtscxmlproc::qtscxmlproc (void) :
    interpreter (),
    _application (nullptr),
    _machine (nullptr),
    _datamodel_name (nullptr),
    _engine (nullptr),
    _monitor (nullptr)
{
}

qtscxmlproc::qtscxmlproc (QCoreApplication* app) :
    interpreter (),
    _application (app),
    _machine (nullptr),
    _datamodel_name (nullptr),
    _engine (nullptr),
    _monitor (nullptr)
{
}

qtscxmlproc::qtscxmlproc (const qtscxmlproc& p) :
    interpreter ()
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
    std::string names = "";
    for (QString name : _machine->stateNames ())
        //qInfo () << ";; state: " << names.at (i).toLocal8Bit().constData () << std::endl;
        names.append (name.toStdString ()).append (" ");
    qInfo () << ";; states:" << names.c_str ();

    assert (_application);
    _machine->setParent (_application);

    // datamodel
    assert (_machine->dataModel ());
    //QScxmlDataModel* datamodel = _machine->dataModel ();
    //assert (datamodel);
    //_machine->setDataModel (new QScxmlEcmaScriptDataModel ());
    _datamodel_name = "null";

    // look up the "datamodel" attribute of the scxml element
    const QString filename = QString (scxml_url.c_str ());
    QFile scxmlfile (filename);
    auto rslt = scxmlfile.open (QIODevice::ReadOnly);
    assert (rslt);
    QXmlStreamReader reader (&scxmlfile);
    while (reader.readNextStartElement ())
    {
        if (!reader.isStartElement ()) continue;
        assert (reader.isStartElement ());
        //qInfo () << "element:" << reader.name ();
        if (reader.name () == "scxml")
        {
            for (QXmlStreamAttribute attr : reader.attributes ())
            {
                if (attr.name () == "datamodel")
                {
                    //qInfo () << "datamodel:" << attr.value ();
                    const char * str = attr.value ().toString ().toStdString ().c_str ();
                    _datamodel_name = strdup (str);
                    break;
                }
            }
            break;
        }
    }
    scxmlfile.close ();
}

// --------------------------------------------------------------------------------
// run
// --------------------------------------------------------------------------------

int
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
    return _application->exec ();

}

// --------------------------------------------------------------------------------
// step
// --------------------------------------------------------------------------------

static nlohmann::json to_nlohmann (const QVariant);
static nlohmann::json to_nlohmann (const QVariantMap);
static nlohmann::json to_nlohmann (const QJsonValue);
static nlohmann::json to_nlohmann (const QJsonObject);
static QVariant to_qvariant (const nlohmann::json);

void
qtscxmlproc::step (void)
{
    qDebug () << ";; step";

    if (!_machine->isRunning ())
    {
        _application->quit ();
        return;
    }
    //assert (_machine->isRunning ());

    QScxmlStateMachinePrivate* smp = ((_QScxmlStateMachine*) _machine)->d ();
    if (smp->m_isProcessingEvents) return;
    // ** note:
    //    when m_isProcessingEvents is true,
    //    queueProcessEvents, called from submitEvent(e), returns without invoking doProcessEvents.
    //    then the event, e, could remain in the queue without being processed.

    assert (_eventin);
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
            usleep (100000);	// 100ms
            // ** this sometimes causes ignoring submitted events.. why?
            return;
        }
        else
            throw ex;
    }

    qInfo () << ";; submitEvent:" << e->name () << e->data () << e->eventType ();
    e->setEventType (QScxmlEvent::InternalEvent);
    _machine->submitEvent (e);
      // submitEvent (e) -> routeEvent (e) -> postEvent (e)
      // -> m_internalQueue.enqueue (e); m_eventLoopHook.queueProcessEvents
    {
        nlohmann::json obj =
            {{"submitEvent", {{"name", e->name ().toStdString ()},
                              {"data", to_nlohmann (e->data ())},
                              {"type", e->eventType ()},
                              {"origin", e->origin ().toStdString ()},
                              {"origintype", e->originType ().toStdString ()},
                              {"sendid", e->sendId ().toStdString ()},
                              {"invokeid", e->invokeId ().toStdString ()}}}};
        _traceout->write (obj);
    }

    //assert (_machine->isRunning ());
    // ** could be paused

    return;
}

// --------------------------------------------------------------------------------
// event in/out
// --------------------------------------------------------------------------------

// jsonstream -> QJsonDocument -> QVariant -> QScxmlEvent
void
qtscxmlproc::event_read (QScxmlEvent& e)
{
    std::string str;
    _eventin->read (str);

    if (str.empty ()) throw std::runtime_error ("Not_found");

    qDebug () << ";; event_read:" << str.c_str ();

    // json parse
    QVariant v = QJsonDocument::fromJson (str.c_str ()).toVariant ();
    assert (v.type () == QVariant::Map);
    QVariantMap map = v.toMap ();

    // event
    assert (v.toMap ().contains ("event"));
    QVariantMap e_map = map["event"].toMap ();

    // target/targetexpr
    // id
    // idlocation
    // delay/delayexpr
    // namelist
    // type/typeexpr (default: http://www.w3.org/TR/scxml/#SCXMLEventProcessor)

    // event keys: name, data, type, origin, origintype, sendid, invokeid

    // event.name
    assert (e_map.contains ("name"));
    QVariant name = e_map["name"];
    assert (name.type () == QVariant::String);
    e.setName (name.toString ());

    // event.data
    QVariant data = e_map.contains ("data") ? e_map["data"] : QVariant ();
    if (data.isValid ()) e.setData (data);

    // event.type
    QVariant type = e_map.contains ("type") ? e_map["type"] : QVariant (QScxmlEvent::ExternalEvent);
    assert (type.canConvert<int> ());
    e.setEventType ((QScxmlEvent::EventType) type.toInt ());

    // event.origin (uri)
    QVariant origin = e_map.contains ("origin") ? e_map["origin"] : QVariant ();
    if (origin.type () == QVariant::String)
    {
        QString str = origin.toString ();
        if (str != "") e.setOrigin (str);
    }

    // event.origintype
    QVariant origintype = e_map.contains ("origintype") ? e_map["origintype"] : QVariant ();
    if (origintype.type () == QVariant::String)
    {
        QString str = origintype.toString ();
        if (str != "") e.setOriginType (str);
    }

    // event.sendid
    QVariant sendid = e_map.contains ("sendid") ? e_map["sendid"] : QVariant ();
    if (sendid.type () == QVariant::String)
    {
        QString str = sendid.toString ();
        if (str != "") e.setSendId (str);
    }

    // event.invokeid
    QVariant invokeid = e_map.contains ("invokeid") ? e_map["invokeid"] : QVariant ();
    if (invokeid.type () == QVariant::String)
    {
        QString str = invokeid.toString ();
        if (str != "") e.setInvokeId (str);
    }
}

// jsonstream -> nlohmann::json -> QScxmlEvent
/*
void
qtscxmlproc::event_read (QScxmlEvent& e)
{
    nlohmann::json obj;
    _eventin->read (obj);

    if (obj.is_null ()) throw std::runtime_error ("Not_found");
    if (obj.find ("event") != obj.end ()) obj = obj["event"];

    qDebug () << ";; event_read:" << obj.dump ().c_str ();

    // name
    assert (obj.count ("name") == 1);
    if (!obj["name"].is_string ()) throw std::runtime_error ("Invalid_argument");
    assert (obj["name"].is_string ());
    std::string name = obj["name"];
    e.setName (QString (name.c_str ()));
    e.setEventType (QScxmlEvent::InternalEvent);	// raise
    //e.setEventType (QScxmlEvent::ExternalEvent);	// send

    // data
    nlohmann::json data = obj["data"];
    QVariant data_var = QVariant ();
    if (obj.count ("data") == 0 || data.is_null ())
    {
        //QJsonValue data_val = QJsonValue ();
        //assert (data_val.isNull ());
        data_var = QVariant ();
        assert (data_var.isNull ());
    }
    else
    {
        data_var = to_qvariant (data);
    }
    //fflush (nullptr);
    assert (data_var.isNull () || data_var.isValid ());
    e.setData (data_var);

    qDebug () << ";; event_read:"
              << "name:" << name.c_str ()
              << "data:" << data_var;
}
*/

// QScxmlEvent -> nlohmann::json -> jsonstream
void
qtscxmlproc::event_write (jsonostream& s, const QScxmlEvent& e)
{
    if (e.eventType () != QScxmlEvent::ExternalEvent) return;

    // keys: name, type, sendid, origin, origintype, invokeid, data

    nlohmann::json obj = {{"event", {{"name", e.name ().toStdString ()}}}};
    s.write (obj);
}

static QScxmlEvent* event_make (const QJsonObject&);

// RAISE
// cf. https://www.w3.org/TR/scxml/#raise
void
qtscxmlproc::js_raise (const QJsonObject& params)
{
    // QJsonObject -> QVariantMap -> QScxmlEvent

    qInfo () << ";; event_raise:" << params;

    QJsonObject obj = params.contains ("event") ? params["event"].toObject () : params;
    QScxmlEvent* e = event_make (obj);
    e->setEventType (QScxmlEvent::InternalEvent);

    assert (_machine);
    _machine->submitEvent (e);
}

// SEND
//
// keys
// - event/eventexpr
// - target/targetexpr
// - type/typeexpr (default: http://www.w3.org/TR/scxml/#SCXMLEventProcessor)
// - id
// - idlocation
// - delay/delayexpr
// - namelist
//
// children
// - param
// - content
//
// cf. https://www.w3.org/TR/scxml/#send
void
qtscxmlproc::js_send (const QJsonObject& params)
{
    // QJsonObject -> std::string (or nlohmann::json) -> jsonstream

    qInfo () << ";; event_send:" << params;
    assert (_eventout);

    assert (params.contains ("event") || params.contains ("eventexpr"));
    // event
    QJsonObject e = params["event"].toObject ();

    // target (uri)
    QString target = "";
    if (params.contains ("target") && params["target"].isString ())
        target = params["target"].toString ();

    // id (xml:id)
    QString id = "";
    if (params.contains ("id") && params["id"].isString ())
        id = params["id"].toString ();

    // idlocation
    QString idlocation = "";
    if (params.contains ("idlocation") && params["idlocation"].isString ())
        idlocation = params["idlocation"].toString ();

    // delay (duration)
    // namelist (list of locations)

    // type (uri)
    QString send_t = "mqtt";
      // short form
      // Processors MAY define short form notations as an authoring convenience (e.g., "scxml" as equivalent to http://www.w3.org/TR/scxml/#SCXMLEventProcessor).
    if (params.contains ("type") && params["type"].isString ())
        send_t = params["type"].toString ();

    if (send_t != "mqtt")
    {
        if (send_t == "scxml" || send_t == "http://www.w3.org/TR/scxml/#SCXMLEventProcessor")
        {
            QScxmlEvent* ev = event_make (e);
            ev->setEventType (QScxmlEvent::ExternalEvent);
            // target can be: #_internal, #_scxml_<sessionid>, #_parent, or #_<invokeid>
            assert (_machine);
            _machine->submitEvent (ev);
        }
        else;
        {
            throw std::runtime_error ("Invalid_argument: " + send_t.toStdString ());
        }
        return;
    }

    assert (send_t == "mqtt");

    // topic (extra parameter for event transmission via mqtt)
    QString topic = "";
    if (params.contains ("topic") && params["topic"].isString ())
        topic = params["topic"].toString ();

    // event emission
#if 0
    std::string str = "{\"event\":" + QJsonValue (e).toString ().toStdString () + "}";
    // ** as with QtScxml 5.9.5, "toString" does not work.
    _eventout->write (str);

#else
    // event keys: name, type, sendid, origin, origintype, invokeid, data

    // name
    assert (e.contains ("name") && e["name"].isString ());
    std::string name = e["name"].toString ().toStdString ();

    // data
    nlohmann::json data =
        (e.contains ("data") && !e["data"].isNull ())? to_nlohmann (e["data"].toVariant ()) : nlohmann::json (nullptr);

    // origin (uri)
    nlohmann::json origin = nullptr;
    if (e.contains ("origin")) origin = e["origin"].toString ().toStdString ();
    // origintype
    nlohmann::json origintype = nullptr;
    if (e.contains ("origintype")) origin = e["origintype"].toString ().toStdString ();
    // sendid
    nlohmann::json sendid = nullptr;
    if (e.contains ("sendid")) origin = e["sendid"].toString ().toStdString ();
    // invokeid
    nlohmann::json invokeid = nullptr;
    if (e.contains ("invokeid")) origin = e["invokeid"].toString ().toStdString ();

    nlohmann::json obj =
        {{"event", {{"name", name},
                    {"data", data},
                    {"type", QScxmlEvent::ExternalEvent},
                    {"origin", origin},
                    {"origintype", origintype},
                    {"sendid", sendid},
                    {"invokeid", invokeid}}},
         {"type", "mqtt"}
        };

    // other "send" params (if exist)
    if (target != "") obj["target"] = target.toStdString ();
    if (id != "") obj["id"] = id.toStdString ();
    if (idlocation != "") obj["idlocation"] = idlocation.toStdString ();
    if (topic != "") obj["topic"] = topic.toStdString ();

    _eventout->write (obj);
#endif

}

// CANCEL
//
// keys
// - sendid/sendidexpr
//
// cf. https://www.w3.org/TR/scxml/#invoke
void
qtscxmlproc::js_cancel (const QJsonObject& params)
{
    assert (!"not yet implemented");
}

// INVOKE
//
// keys
// - src/srcexpr
// - type/typeexpr
// - id
// - idlocation
// - delay/delayexpr
// - namelist
// - autoforward
//
// children
// - param
// - finalize
// - content
//
// cf. https://www.w3.org/TR/scxml/#invoke
void
qtscxmlproc::js_invoke (const QJsonObject& params)
{
    assert (params.contains ("src") || params.contains ("srcexpr"));

    assert (!"not yet implemented");
}

// {"name" : name, "data" : data, ... } -> QScxmlEvent
// cf. https://www.w3.org/TR/scxml/#InternalStructureofEvents
static QScxmlEvent*
event_make (const QJsonObject& obj)
{
    QScxmlEvent* e = new QScxmlEvent ();
    // event keys: name, data, type, origin, origintype, sendid, invokeid

    // name. This is a character string giving  the name of the event. The SCXML
    // Processor MUST set the  name field to the name of this  event. It is what
    // is matched against the 'event' attribute of <transition>.
    // Note that transitions can do additional  tests by using the value of this
    // field inside boolean expressions in the 'cond' attribute.
    assert (obj.contains ("name") && obj["name"].isString ());
    e->setName (obj["name"].toString ());

    // type. This field describes the event type. The SCXML Processor MUST set it to: 
    // "platform" (for events raised by the platform itself, such as error events)
    // "internal" (for events raised by <raise> and <send> with target '_internal')
    // "external" (for all other events).
    if (obj.contains ("type"))
    {
        int n = obj["type"].toInt ();
        assert (0 <= n && n <= 2); // Platform = 0, Internal, External
        e->setEventType (QScxmlEvent::EventType (n));
    }

    if (obj.contains ("data"))
        e->setData (obj["data"]);

    // origin. This is a URI, equivalent to the 'target' attribute on the <send>
    // element.
    // For external events, the SCXML Processor SHOULD set this field to a value
    // which, when used as the value of 'target', will allow the receiver of the
    // event to <send>  a response back to the originating  entity via the Event
    // I/O Processor specified in 'origintype'.
    // For internal and platform events, the Processor MUST leave this field blank.
    if (obj.contains ("origin") && obj["origin"].isString ())
        e->setOrigin (obj["origin"].toString ());
    if (obj.contains ("origintype") && obj["origintype"].isString ())
        e->setOriginType (obj["origintype"].toString ());

    // sendid.  If the  sending  entity  has specified  a  value  for this,  the
    // Processor MUST set  this field to that value (see  C Event I/O Processors
    // for  details). Otherwise,  in the  case of  error events  triggered by  a
    // failed attempt to send an event, the Processor MUST set this field to the
    // send id  of the  triggering <send>  element.
    // Otherwise  it MUST  leave it blank.
    if (obj.contains ("sendid") && obj["sendid"].isString ())
        e->setSendId (obj["sendid"].toString ());

    // invokeid. If this  event is generated from an invoked  child process, the
    // SCXML Processor  MUST set this field  to the invoke id  of the invocation
    // that triggered the child process.
    // Otherwise it MUST leave it blank.
    if (obj.contains ("invokeid") && obj["invokeid"].isString ())
        e->setSendId (obj["invokeid"].toString ());

    return (e);
}

// --------------------------------------------------------------------------------
// conversion: nlohmann::json <-> qt
// --------------------------------------------------------------------------------

static QVariant to_qvariant (const nlohmann::json j)
{
    qDebug () << ";; to_qvariant (json):" << j.dump ().c_str ();

    QVariant data_var;
    if (j.is_null ())
    {
        QJsonValue data_val = QJsonValue ();
        assert (data_val.isNull ());
        return (data_val.toVariant ());
    }
    else if (j.is_string ())
    {
        std::string str = j;
        QJsonValue data_val = QJsonValue (str.c_str ());
        //data_val = QJsonValue (data.dump ().c_str ());
        return (data_val.toVariant ());
    }
    else if (j.is_number ())
    {
        qDebug () << ";; to_qvariant (json.number)";
        if (j.is_number_integer ())
        {
            int i = j;
            return (QVariant (i));
        }
        else if (j.is_number_unsigned ())
        {
            unsigned int i = j;
            return (QVariant (i));
        }
        else if (j.is_number_float ())
        {
            float f = j;
            return (QVariant (f));
        }
        assert (false);
        return (QVariant ());
    }
    else if (j.is_object ())
    {
        std::string str = j.dump ();
        //const char* str = data.get<std::string> ().c_str ();
        qDebug () << ";; event_read (object):" << str.c_str ();
        //data_val = QJsonValue (str);
        return (QJsonDocument::fromJson (str.c_str ()).toVariant ());
    }
    else
    {
        const char* str = j.dump ().c_str ();
        qDebug () << ";; event_read (unknown):" << str;
        return (QJsonDocument::fromJson (str).toVariant ());
    }
}

static nlohmann::json
to_nlohmann (const QVariant v)
{
    qDebug () << ";; to_nlohmann (QVariant):" << v << v.typeName ();

    if (v.isNull ()) return (nullptr);

    switch (v.type ())
    {
    case QVariant::String:
        return nlohmann::json (v.toString ().toStdString ());
    case QVariant::Int:
        return nlohmann::json (v.toInt ());
    case QVariant::Double:
        return nlohmann::json (v.toDouble ());
    case QVariant::Map:
        // object
        //return to_nlohmann (v.toJsonObject ());
        return to_nlohmann (v.toMap ());
    default:
        qDebug () << ";; unknown QVariant type";
    }

    return (to_nlohmann (v.toJsonValue ()));
}

static nlohmann::json
to_nlohmann (const QVariantMap map)
{
    qDebug () << ";; to_nlohmann (QVariantMap):" << map;

    nlohmann::json j = {};
    for (QString key : map.keys ())
    {
        j[key.toStdString()] = to_nlohmann (map[key]);
    }

    return (j);
}

static nlohmann::json
to_nlohmann (const QJsonValue v)
{
    qDebug () << ";; to_nlohmann (QJsonValue):" << v;
    if (v.isNull ())
    {
        qDebug () << ";; to_nlohmann (null)";
        return (nlohmann::json (nullptr));
    }
    else if (v.isString ())
    {
        std::string str = v.toString ().toStdString ();
        qDebug () << ";; to_nlohmann (QJsonValue.string);" << str.c_str ();
        //qInfo () << ";; to_nlohmann:" << v.toString () << v.toString ().size ();
        return (nlohmann::json (str));
    }
    else if (v.isObject ())
    {
        qDebug () << ";; to_nlohmann (QJsonValue.object)";
    }

    return (nlohmann::json (v.toString ().toStdString ()));
}

static nlohmann::json
to_nlohmann (const QJsonObject obj)
{
    qDebug () << ";; to_nlohmann (QJsonObject):" << obj;

    nlohmann::json j = {};
    for (QString key : obj.keys ())
    {
        const QJsonValue elt = obj[key];
        j[key.toStdString()] = to_nlohmann (elt);
    }
    return (j);
}

// --------------------------------------------------------------------------------
// monitor
// --------------------------------------------------------------------------------

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

// --------------------------------------------------------------------------------
// setup
// --------------------------------------------------------------------------------

void
qtscxmlproc::verbosity_set (int v)
{
    QLoggingCategory* default_cat = QLoggingCategory::defaultCategory ();
    //qDebug () << QStandardPaths::standardLocations (QStandardPaths::GenericConfigLocation);
    switch (v)
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
        QLoggingCategory::setFilterRules ("default.debug=false\n"
                                          "qt.scxml.statemachine.debug=false\n");
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
                                  qDebug () << ";; event:" << e.name ()
                                            << e.origin () << e.originType ()
                                            << e.scxmlType ()
                                            << e.eventType ();
                                  //event_write (*_eventout, e);
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
                          qInfo () << ";; SIGNAL: finished";
                          _application->quit (); 
                      });

    // SIGNAL: initializedChanged (initialized)
    QObject::connect (_machine, &QScxmlStateMachine::initializedChanged,
                      [this](bool initialized) {
                          qInfo () << ";; SIGNAL: initializedChanged" << initialized;
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
                          qInfo () << ";; SIGNAL: reachedStableState";
                          step ();
                      });

    // SIGNAL: runningChanged (bool running)
    QObject::connect (_machine, &QScxmlStateMachine::runningChanged,
                      [this](bool running) {
                          qInfo () << ";; SIGNAL: runningChanged: " << running;
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


    // init
    _machine->init ();

    // hack
    _hack ();
}

void
qtscxmlproc::version (void)
{
    std::cerr << "ScxmlRun\t\t"
              << SCXMLRUN_VERSION
              << std::endl;

    std::cerr << "QtCore\t\t\t"
              << QT_VERSION_STR
              << std::endl;

    std::cerr << "Mosquitto\t\t"
              << LIBMOSQUITTO_MAJOR << "."
              << LIBMOSQUITTO_MINOR << "."
              << LIBMOSQUITTO_REVISION
              << std::endl;

    auto json_meta = nlohmann::json::meta ();
    std::cerr << "JSON for C++\t\t"
              << json_meta["version"]["major"] << "."
              << json_meta["version"]["minor"] << "."
              << json_meta["version"]["patch"]
              << std::endl;
}

void
qtscxmlproc::_hack (void)
{
    assert (_datamodel_name);
    qInfo () << ";; datamodel:" << _datamodel_name;
    if (strcmp (_datamodel_name, "ecmascript") != 0) return;

    QScxmlDataModel* datamodel = _machine->dataModel ();
    assert (datamodel && datamodel->stateMachine () == _machine);
    _QScxmlEcmaScriptDataModel* _datamodel = (_QScxmlEcmaScriptDataModel*)datamodel;
    if (!_datamodel->d ()->engine ()) return;
    assert (!_engine);

    // set a JSEngine object to _engine
    _engine = _datamodel->d ()->assertEngine ();
    // ** assertEngine () creates an engine object if _datamodel does not carry one.
    assert (_engine);

    QJSValue global = _engine->globalObject ();
    assert (global.isObject ());

    // system variables: _event, _sessionid, _name, _ioprocessors, _x
    // https://www.w3.org/TR/scxml/#SystemVariables
    QJSValue _sessionid = global.property ("_sessionid");
    qInfo () << ";; _sessionid:" << _sessionid.toString ();

    QJSValue _name = global.property ("_name");
    qInfo () << ";; _name:" << _name.toString ();

    // _ioprocessors:
    // - http://www.w3.org/TR/scxml/#SCXMLEventProcessor (default)
    // - http://www.w3.org/TR/scxml/#BasicHTTPEventProcessor (optional) -- not supported by QtSCXML
    QJSValue _ioprocessors = global.property ("_ioprocessors");
    assert (_ioprocessors.isObject () && _ioprocessors.hasProperty ("scxml"));
    qInfo () << ";; _ioprocessors.scxml:" << _ioprocessors.property ("scxml").toString ();
    QJSValue scxml_loc = _engine->evaluate ("_ioprocessors.scxml.location");
    qInfo () << ";; _ioprocessors.scxml.location:" << scxml_loc.toString ();
    // QtSCXML only supports: <send type="http://www.w3.org/TR/scxml/#SCXMLEventProcessor"...>

    // JSEngine extension
    QJSEngine::Extensions exts = QJSEngine::TranslationExtension | QJSEngine::ConsoleExtension;
    _engine->installExtensions (exts);

    //_JSScxml* scxml = new _JSScxml ();
    //scxml->_proc = this;
    //QJSValue scxml_val = _engine->newQObject (scxml);
    //global.setProperty ("SCXML", scxml_val);
    _JSScxml::intern (_engine, this);

    // declare _data as a global variable
    if (!global.hasProperty ("_data"))
    {
        _engine->evaluate ("_data = {}");
    }
    assert (global.hasProperty ("_data"));

    // console
    //QJSEngine::Extensions exts = QJSEngine::TranslationExtension | QJSEngine::ConsoleExtension;
    //_engine->installExtensions (exts);
#if 0
    // console.log
    //_JSConsole* console = new _JSConsole ();
    //QJSValue console_val = _engine->newQObject (console);
    //global.setProperty ("console", console_val);
    _JSConsole::intern (_engine);
#endif

}
