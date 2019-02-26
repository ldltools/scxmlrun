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

#include "jsonstream.hpp"

#include <cassert>
#include <fstream>
#include <stdexcept>
#include <unistd.h>

// ================================================================================
// jsonifstream
// ================================================================================

jsonifstream::jsonifstream (void) :
    _in (nullptr)
{
}

jsonifstream::jsonifstream (const char* filename)
{
    _in = new std::ifstream (filename, std::ifstream::in);
}

jsonifstream::~jsonifstream (void)
{
}

jsonifstream&
jsonifstream::read (std::string& obj)
{
    assert (_in);

    char str[0x4000];	// 4KB
    _in->getline (str, 0x4000);
    if (_in->eof ())
        throw std::runtime_error ("End_of_file");
    if (_in->fail ())
        throw std::runtime_error ("Failure: jsonifstream::read");

    assert (obj.empty ());
    obj += str;

    return (*this);
}

jsonifstream&
jsonifstream::read (nlohmann::json& obj)
{
    assert (_in);

    obj = nullptr;

    char str[0x4000];	// 4KB
    _in->getline (str, 0x4000);
    if (_in->eof ())
        throw std::runtime_error ("End_of_file");
    if (_in->fail ())
        throw std::runtime_error ("Failure: jsonifstream::read");

    //std::clog << ";; read: '" << str << "'" << std::endl;
    obj = nlohmann::json::parse (str);

    return (*this);
}

// ================================================================================
// jsonimstream
// ================================================================================

jsonimstream::jsonimstream (void) :
    _mosq (nullptr)
{
}

jsonimstream::~jsonimstream (void)
{
    if (_mosq)
    {
        mosquitto_disconnect (_mosq);
        mosquitto_destroy (_mosq);
    }
}

jsonimstream&
jsonimstream::read (std::string& obj)
{
    assert (obj.empty ());
    if (_messages.empty ()) return (*this);
    assert (!_messages.empty ());

    std::string* msg = _messages.front ();
    _messages.pop ();

    obj += msg->c_str ();
    delete msg;

    return (*this);
}

jsonimstream&
jsonimstream::read (nlohmann::json& obj)
{
    //std::clog << ";; read\n";

    if (_messages.empty ())
    {
        obj = nullptr;
        return (*this);
    }
    /*
    while (_messages.empty ())
    {
        //std::clog << ";; sleep\n";
        usleep (100000);	// 100ms
    }
    */

    assert (!_messages.empty ());
    std::string* msg = _messages.front ();
    _messages.pop ();
    obj = nlohmann::json::parse (*msg);
    delete msg;

}

// ================================================================================
// jsonofstream
// ================================================================================

jsonofstream::jsonofstream (void) :
    _out (nullptr)
{
}

jsonofstream::jsonofstream (const char* filename)
{
    _out = new std::ofstream (filename, std::ofstream::out | std::ofstream::app);
}

jsonofstream::~jsonofstream (void)
{
    if (_out) delete _out;
}

jsonofstream&
jsonofstream::write (nlohmann::json& obj)
{
    assert (_out);
    std::string str = obj.dump ();

    return (write (str));

    /*
    //std::clog << ";; write: '" << str << "'" << std::endl;
    *_out << str << std::endl;
    return (*this);
    */
}

jsonofstream&
jsonofstream::write (const std::string& str)
{
    assert (_out);
    //std::cerr << ";; write: '" << str << "'" << std::endl;
    *_out << str << std::endl;

    return (*this);
}

// ================================================================================
// jsonomstream
// ================================================================================

jsonomstream::jsonomstream (void) :
    _mosq (nullptr),
    _topic (nullptr)
{
}

jsonomstream::~jsonomstream (void)
{
    if (_mosq)
    {
        mosquitto_disconnect (_mosq);
        mosquitto_destroy (_mosq);
    }
}

jsonomstream&
jsonomstream::write (nlohmann::json& obj)
{
    assert (obj.is_object ());
    //assert (obj["type"] == "mqtt");

    std::string msg = obj.dump ();

    const char* topic = nullptr;
    if (obj.find ("topic") != obj.end ())
    {
        nlohmann::json topic_val = obj["topic"];
        if (topic_val.is_string ())
        {
            std::string topic_str = topic_val;
            topic = topic_str.c_str ();
        }
    }

    return (write (msg.c_str (), topic, 1));
}

jsonomstream&
jsonomstream::write (const std::string& msg)
{
    return (write (msg.c_str (), _topic, 1));
}

jsonomstream&
jsonomstream::write (const char* msg, const char* topic, int qos)
{
    assert (msg);
    int len = strlen (msg);
    const char* topic_ = (topic && strlen (topic) > 0) ? topic : _topic;
    assert (topic_);
    assert (0 <= qos && qos <= 2);

    assert (_mosq);
    int rslt = mosquitto_publish (_mosq, nullptr, topic_, len, msg, qos, false);
    assert (rslt == MOSQ_ERR_SUCCESS);

    return (*this);
}
