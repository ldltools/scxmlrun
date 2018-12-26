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

#ifndef QTSCXMLPROC_HPP
#define QTSCXMLPROC_HPP

#include "scxmlinterpreter.hpp"
#include <QtCore/QCoreApplication>
#include <QtScxml/QScxmlStateMachine>

// for hacking QScxmlEcmaScriptDataModel
#include <QtCore/qglobal.h>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#include <QtScxml/QScxmlEcmaScriptDataModel>
#include <QtScxml/qscxmldatamodel.h>
#include <QtScxml/private/qscxmldatamodel_p.h>
#include <QtScxml/qscxmlecmascriptdatamodel.h>

namespace scxml {

class monitor;

class qtscxmlproc : public scxmlproc
{
public:
    virtual void load (const std::string& scxml_url);
    virtual void setup (void);
    virtual int run (void);

public:
    void step (void);

public:
    void event_read (QScxmlEvent&);
    void event_write (jsonostream&, const QScxmlEvent&);
      // _event keys: name, type, sendid, origin, origintype, invokeid, data
      // https://www.w3.org/TR/scxml/#InternalStructureofEvents

public:
    void js_raise (const QJsonObject& params);
      // params = {name: <name>, ..event_attrs..}
    void js_send (const QJsonObject& params);
      // params = {event: <event>, ..send_opts..}
    void js_cancel (const QJsonObject& params);
    void js_invoke (const QJsonObject& params);

public:
    qtscxmlproc (void);
    qtscxmlproc (QCoreApplication* app = nullptr);
    qtscxmlproc (const qtscxmlproc&);
    ~qtscxmlproc (void);

public:
    virtual void verbosity_set (int);
    static void version (void);

protected:
    QCoreApplication* _application;
    QScxmlStateMachine* _machine;
    const char* _datamodel_name;
    QJSEngine* _engine;
    monitor* _monitor;

private:
    void _hack (void);
};

//
class monitor : public QObject
{
Q_OBJECT

public:
    //monitor (void) : QObject () {}
    //explicit monitor (monitor&) : QObject () {}
    //explicit monitor (void) : QObject () {}
    monitor (QObject* parent = 0) : QObject (parent) {}
    //monitor (QObject* parent = 0) : QObject (parent) {}
    //explicit monitor (QObject* parent = 0);

public:
    void state_cb (const QString&, bool active);

signals:
public slots:
    //void log (const QString&, const QString&);

private:
    qtscxmlproc* _proc;
    jsonostream* _traceout;

    friend class qtscxmlproc;
};

}

//
class _QScxmlStateMachine : public QScxmlStateMachine
{
public:
    QScxmlStateMachinePrivate* d () const
    {
        return (reinterpret_cast<QScxmlStateMachinePrivate*>(d_ptr.data ()));
    }
};

//
class _QScxmlEcmaScriptDataModelPrivate : public QScxmlDataModelPrivate
{
    Q_DECLARE_PUBLIC(QScxmlEcmaScriptDataModel)
public:
    _QScxmlEcmaScriptDataModelPrivate()
        : jsEngine(Q_NULLPTR)
    {}

    QJSEngine* assertEngine ()
    {
        if (!jsEngine)
        {
            Q_Q (QScxmlEcmaScriptDataModel);
            jsEngine = new QJSEngine (q->stateMachine ());
        }
        return jsEngine;
    }       

    QJSEngine* engine () const
    {
        return (jsEngine);
    }

public:
    QStringList initialDataNames;

private:
    QJSEngine *jsEngine;
    QJSValue dataModel;
};

//
class _QScxmlEcmaScriptDataModel : public QScxmlEcmaScriptDataModel
{
    _QScxmlEcmaScriptDataModel () {}
public:
    _QScxmlEcmaScriptDataModelPrivate* d () const
    {
        return (reinterpret_cast<_QScxmlEcmaScriptDataModelPrivate*>(d_ptr.data ()));
    }
};

//
class _JSScxml : public QObject
{
    Q_OBJECT

public:
    explicit _JSScxml (QObject* parent = nullptr) : QObject (parent) {}

public slots:
    void _raise (const QString);
    void _send (const QString);
    void _cancel (const QString);
    void _invoke (const QString);

private:
    scxml::qtscxmlproc* _proc;

    friend class scxml::qtscxmlproc;
};

//
class _JSConsole : public QObject
{
    Q_OBJECT
public:
    explicit _JSConsole (QObject* parent = nullptr) : QObject (parent) {}

signals:
public slots:
    void _log (const QString);
};

#endif
