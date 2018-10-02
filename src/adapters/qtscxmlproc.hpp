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

#include "scxmlproc.hpp"
#include <QtCore/QCoreApplication>
#include <QtScxml/QScxmlStateMachine>

// for hacking QScxmlEcmaScriptDataModel
#include <QtCore/qglobal.h>
#include <QtQml/QJSEngine>
#include <QtScxml/QScxmlEcmaScriptDataModel>
#include <QtScxml/private/qscxmldatamodel.h>
#include <QtScxml/private/qscxmldatamodel_p.h>
#include <QtScxml/private/qscxmlecmascriptdatamodel.h>

namespace scxml {

class monitor;

class qtscxmlproc : public scxmlproc
{
public:
    virtual void load (const std::string& scxml_url);
    virtual void setup (void);
    virtual void run (void);

    static void version (void);

public:
    void step (const bool timeout = false);

public:
    void event_read (QScxmlEvent&);
    void event_write (jsonostream&, const QScxmlEvent&);

public:
    virtual void verbosity_set (int);

public:
    qtscxmlproc (void);
    qtscxmlproc (QCoreApplication* app = nullptr);
    qtscxmlproc (const qtscxmlproc&);
    ~qtscxmlproc (void);

protected:
    QCoreApplication* _application;
    QScxmlStateMachine* _machine;
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

//
class JSConsole : public QObject
{
    Q_OBJECT
public:
    explicit JSConsole (QObject* parent = nullptr) : QObject (parent) {}

signals:

public slots:
    void log (const QString);
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
        //if (!jsEngine)
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

}

#endif
