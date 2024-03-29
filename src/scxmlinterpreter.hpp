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

#ifndef SCXMLINTERPRETER_HPP
#define SCXMLINTERPRETER_HPP

#include "jsonstream.hpp"
#include <mosquitto.h>

#include <list>
#include <ostream>

namespace scxml {

class interpreter
{
public:
    virtual void load (const std::string& scxml_url) = 0;
    virtual void setup (void) = 0;
    virtual int run (void) = 0;

public:
    jsonistream& eventin (void) { return (*_eventin); }
    //void eventin (jsonistream& s) { _eventin = &s; }
    void eventin_open (const char* filename);
    void eventin_open (mosquitto*, std::list<const char*>&);

    void eventout_open (const char* filename);
    void eventout_open (mosquitto*, const char*);

    void traceout (jsonostream& s) { _traceout = &s; }
    void traceout_open (const char* filename);
    void traceout_open (mosquitto*, const char*);

public:
    void mosq_set_callbacks (mosquitto*);

private:
    static void eventin_message_cb (mosquitto*, void*, const mosquitto_message*);
    static void mosq_disconnect_cb (mosquitto*, void*, int);
    static void mosq_publish_cb (mosquitto*, void*, int);

protected:
    jsonistream* _eventin;
    jsonostream* _eventout;
    jsonostream* _traceout;

public:
    interpreter (void);
    interpreter (const interpreter&);
    ~interpreter (void);

public:
    virtual void verbosity_set (int v) = 0;
      // -1: silent
      //  0: default
      //  1: verbose
      //  2: extra-verbose

    std::ostream& cout (void) { assert (_cout); return (*_cout); }
    std::ostream& cerr (void) { assert (_cerr); return (*_cerr); }
    void cout (std::ostream& s) { _cout = &s; }
    void cerr (std::ostream& s) { _cerr = &s; }

protected:
    std::ostream* _cout;		// stdout
    std::ostream* _cerr;		// stderr

};

}

#endif
