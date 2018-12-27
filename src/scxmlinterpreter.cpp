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

#include "scxmlinterpreter.hpp"

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

interpreter::interpreter (void) :
    _eventin (nullptr),
    _eventout (nullptr),
    _traceout (nullptr),
    _cout (&std::cout),
    _cerr (&std::cerr)
{
}

interpreter::interpreter (const interpreter& p) :
    interpreter ()
{
    assert (false);
}

interpreter::~interpreter (void)
{
    if (_eventin) delete _eventin;
    if (_traceout) delete _traceout;
}

// --------------------------------------------------------------------------------
// eventin
// --------------------------------------------------------------------------------

void
interpreter::eventin_open (const char* filename)
{
    assert (!_eventin);

    if (!filename) return;

    _eventin = new jsonifstream (filename);
}

void
interpreter::eventin_open (mosquitto* mosq, std::list<const char*>& topics)
{
    assert (mosq);
    if (topics.size () == 0) return;

    assert (!_eventin);
    jsonimstream* s = new jsonimstream (mosq);
    _eventin = s;

    for (auto it = topics.begin (); it != topics.end (); it++)
    {
        int qos = 1;
        int rslt = mosquitto_subscribe (mosq, nullptr, *it, qos);
        assert (rslt == MOSQ_ERR_SUCCESS);
        //std::clog << ";; subscribe: " << *it << std::endl;
    }

}

// --------------------------------------------------------------------------------
// eventout
// --------------------------------------------------------------------------------

void
interpreter::eventout_open (const char* filename)
{
    assert (!_eventout);

    if (!filename) return;

    _eventout = new jsonofstream (filename);
}

void
interpreter::eventout_open (mosquitto* mosq, const char* topic)
{
    assert (mosq);
    if (!topic) return;

    //mosq_set_callbacks (mosq);
    //mosquitto_user_data_set (mosq, this);

    _eventout = new jsonomstream (mosq, topic);
}

// --------------------------------------------------------------------------------
// traceout
// --------------------------------------------------------------------------------

void
interpreter::traceout_open (const char* filename)
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

void
interpreter::traceout_open (mosquitto* mosq, const char* topic)
{
    assert (mosq);
    if (!topic) return;

    assert (!_traceout);
    jsonomstream* s = new jsonomstream (mosq, topic);
    _traceout = s;

}

// --------------------------------------------------------------------------------
// mosquitto callbacks
// --------------------------------------------------------------------------------

void
interpreter::mosq_set_callbacks (mosquitto* mosq)
{
    mosquitto_user_data_set (mosq, this);
    mosquitto_disconnect_callback_set (mosq, mosq_disconnect_cb);
    mosquitto_message_callback_set (mosq, eventin_message_cb);
    mosquitto_publish_callback_set (mosq, mosq_publish_cb);
}

void
interpreter::eventin_message_cb (mosquitto* mosq, void* obj, const mosquitto_message* msg)
{
    //std::clog << ";; message[" << msg->topic << "]: ";
    const int len = msg->payloadlen;
    std::string* str = new std::string ((const char*) msg->payload, 0, len);
    //char str[len + 1];
    //memcpy (str, msg->payload, len); str[len] = '\0';
    //std::clog << *str << std::endl;

    assert (obj);
    interpreter* proc = (interpreter*) obj;	// ** insecure
    assert (proc);
    jsonimstream* s = (jsonimstream*) proc->_eventin;
    assert (s && &s->broker () == mosq);
    s->messages (). push (str);
    assert (!s->messages (). empty ());
}

void
interpreter::mosq_disconnect_cb (mosquitto* mosq, void* obj, int rc)
{
    //std::cerr << ";; mosquitto on_disconnect " << rc << std::endl;
}

void
interpreter::mosq_publish_cb (mosquitto* mosq, void* obj, int mid)
{
    //std::cerr << ";; mosquitto on_publish " << mid << std::endl;
}
