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

#ifndef ESPRIMITIVES_HPP
#define ESPRIMITIVES_HPP

#include "qtscxmlproc.hpp"

#include <QtCore/QCoreApplication>
#include <QtScxml/QScxmlStateMachine>

// for hacking QScxmlEcmaScriptDataModel
#include <QtCore/qglobal.h>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#include <QtScxml/QScxmlEcmaScriptDataModel>
#include <QtScxml/QScxmlDataModel>

// global "SCXML" object
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

public:
    static void intern (QJSEngine*, scxml::qtscxmlproc*);
    // intern: SCXML.raise/send/cancel/invoke

private:
    scxml::qtscxmlproc* _proc;

    friend class scxml::qtscxmlproc;
};

// global "console" object
class _JSConsole : public QObject
{
    Q_OBJECT
public:
    explicit _JSConsole (QObject* parent = nullptr) : QObject (parent) {}

public:
    static void intern (QJSEngine*);
     // intern: console.log

signals:
public slots:
    void _log (const QString);
};

#endif
