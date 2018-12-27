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

#include "esprimitives.hpp"
#include "moc_esprimitives.hpp"

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
// _JSScxml: global "SCXML" object
// --------------------------------------------------------------------------------

void
_JSScxml::_raise (const QString str)
{
    QJsonObject obj = QJsonDocument::fromJson (str.toUtf8 ()).object ();

    //QJSValue js_val (str);
    //qInfo () << ";; _JSScxml::raise:" << js_val.toString ();
    //QVariant v = js_val.toVariant ();
    // cf. http://doc.qt.io/qt-5/qjsvalue.html#toVariant

    assert (_proc);
    _proc->js_raise (obj);
}

void
_JSScxml::_send (const QString str)
{
    QJsonObject obj = QJsonDocument::fromJson (str.toUtf8 ()).object ();

    //QJSValue js_val (str);
    //qInfo () << ";; _JSScxml::send:" << js_val.toString ().toStdString ().c_str ();
    //QVariant v = js_val.toVariant ();
    // cf. http://doc.qt.io/qt-5/qjsvalue.html#toVariant
    assert (_proc);
    _proc->js_send (obj);
}

void
_JSScxml::_cancel (const QString str)
{
    QJsonObject obj = QJsonDocument::fromJson (str.toUtf8 ()).object ();
    assert (_proc);
    _proc->js_cancel (obj);
}

void
_JSScxml::_invoke (const QString str)
{
    QJsonObject obj = QJsonDocument::fromJson (str.toUtf8 ()).object ();
    assert (_proc);
    _proc->js_invoke (obj);
}

void
_JSScxml::intern (QJSEngine* e, scxml::qtscxmlproc* p)
{
    assert (e && p);

    _JSScxml* scxml = new _JSScxml ();
    scxml->_proc = p;
    QJSValue scxml_val = e->newQObject (scxml);
    QJSValue global = e->globalObject ();
    assert (global.isObject ());
    assert (!global.hasProperty ("SCXML"));
    global.setProperty ("SCXML", scxml_val);
    // scxml_val points to the global "SCXML" object

    // SCXML.raise
    QJSValue raise_fun = e->evaluate ("function (obj) { SCXML._raise (JSON.stringify (obj)) }");
    assert (raise_fun.isCallable ());
    scxml_val.setProperty ("raise", raise_fun);
    // SCXML.send
    QJSValue send_fun = e->evaluate ("function (obj) { SCXML._send (JSON.stringify (obj)) }");
    assert (send_fun.isCallable ());
    scxml_val.setProperty ("send", send_fun);
    // SCXML.cancel
    QJSValue cancel_fun = e->evaluate ("function (obj) { SCXML._cancel (JSON.stringify (obj)) }");
    assert (cancel_fun.isCallable ());
    scxml_val.setProperty ("cancel", cancel_fun);
    // SCXML.invoke
    QJSValue invoke_fun = e->evaluate ("function (obj) { SCXML._invoke (JSON.stringify (obj)) }");
    assert (invoke_fun.isCallable ());
    scxml_val.setProperty ("invoke", invoke_fun);
}

// --------------------------------------------------------------------------------
// _JSConsole: global "console" object
// --------------------------------------------------------------------------------

void
_JSConsole::_log (const QString msg)
{
    std::string str = msg.toStdString ();
    qDebug () << "jsConsole: "<< str.c_str () << msg.size ();
}

void
_JSConsole::intern (QJSEngine* e)
{
    assert (e);
    QJSValue global = e->globalObject ();
    assert (global.isObject ());
    if (global.hasProperty ("console")) return;

    _JSConsole* console = new _JSConsole ();
    QJSValue console_val = e->newQObject (console);
    global.setProperty ("console", console_val);
    // console_val points to the global "console" object

    QJSValue log_fun = e->evaluate ("function (obj) { console._log (JSON.stringify (obj)) }");
    console_val.setProperty ("log", log_fun);
}
