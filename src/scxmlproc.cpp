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

#include "scxmlproc.hpp"

#include <cassert>
#include <codecvt>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

using namespace scxml;

// --------------------------------------------------------------------------------
// ctor/dtor
// --------------------------------------------------------------------------------

scxmlproc::scxmlproc (void) :
    _eventin (nullptr),
    _eventout (nullptr),
    _traceout (nullptr),
    _cout (&std::cout),
    _cerr (&std::cerr)
{
}

scxmlproc::scxmlproc (const scxmlproc& p) :
    scxmlproc ()
{
    assert (false);
}

scxmlproc::~scxmlproc (void)
{
    if (_eventin) delete _eventin;
    if (_traceout) delete _traceout;
}

// --------------------------------------------------------------------------------
// eventin
// --------------------------------------------------------------------------------

void
scxmlproc::eventin_open (const char* filename)
{
    assert (!_eventin);

    if (!filename) return;

    _eventin = new jsonifstream (filename);
}

void
scxmlproc::eventin_open (mosquitto* mosq, std::list<const char*>& topics)
{
    assert (mosq);
    if (topics.size () == 0) return;

    assert (!_eventin);
    jsonimstream* s = new jsonimstream (mosq);
    _eventin = s;

    for (auto it = topics.begin (); it != topics.end (); it++)
    {
        int rslt = mosquitto_subscribe (mosq, nullptr, *it, 0);
        assert (rslt == MOSQ_ERR_SUCCESS);
        //std::clog << ";; subscribe: " << *it << std::endl;
    }

    mosquitto_user_data_set (mosq, _eventin);
    mosquitto_message_callback_set (mosq, eventin_message_cb);
}

void
scxmlproc::eventin_message_cb (mosquitto* mosq, void* obj, const mosquitto_message* msg)
{
    //std::clog << ";; message[" << msg->topic << "]: ";
    const int len = msg->payloadlen;
    std::string* str = new std::string ((const char*) msg->payload, 0, len);
    //char str[len + 1];
    //memcpy (str, msg->payload, len); str[len] = '\0';
    //std::clog << *str << std::endl;

    assert (obj);
    jsonimstream* s = (jsonimstream*) obj;	// ** insecure
    assert (&s->broker () == mosq);
    s->messages (). push (str);
    assert (!s->messages (). empty ());

}

// --------------------------------------------------------------------------------
// eventout
// --------------------------------------------------------------------------------

void
scxmlproc::eventout_open (const char* filename)
{
    assert (!_eventout);

    if (!filename) return;

    _eventout = new jsonofstream (filename);
}

void
scxmlproc::eventout_open (mosquitto* mosq, const char* topic)
{
    assert (mosq);
    if (!topic) return;
}

// --------------------------------------------------------------------------------
// traceout
// --------------------------------------------------------------------------------

void
scxmlproc::traceout_open (const char* filename)
{
    assert (!_traceout);

    if (!filename) return;
    //if (_traceout.is_open ()) return;

    jsonofstream* fs = new jsonofstream (filename);
    _traceout = fs;
    //std::clog << ";; trace: " << filename << std::endl;
    //out->open (filename, std::fstream::out | std::fstream::app);
    //assert (fs->is_open () && fs->good ());
}
/*
void
scxmlproc::out_open (std::ostream*& out, const char* filename)
{
    assert (!out);

    if (!filename) return;
    //if (_traceout.is_open ()) return;

    std::fstream* fs = new std::fstream (filename, std::fstream::out | std::fstream::app);
    out = fs;
    std::clog << ";; trace: " << filename << std::endl;
    //out->open (filename, std::fstream::out | std::fstream::app);
    assert (fs->is_open () && fs->good ());
}
*/

void
scxmlproc::traceout_open (mosquitto* mosq, const char* topic)
{
    assert (mosq);
    if (!topic) return;

    assert (!_traceout);
    jsonomstream* s = new jsonomstream (mosq);
    _traceout = s;

    /*
    for (auto it = topics.begin (); it != topics.end (); it++)
    {
        int rslt = mosquitto_subscribe (mosq, nullptr, *it, 0);
        assert (rslt == MOSQ_ERR_SUCCESS);
        std::clog << ";; subscribe: " << *it << std::endl;
    }

    mosquitto_user_data_set (mosq, _traceoute);
    mosquitto_message_callback_set (mosq, eventin_message_cb);
    */
}
