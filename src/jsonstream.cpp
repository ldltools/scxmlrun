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
jsonifstream::read (nlohmann::json& obj)
{
    assert (_in);

    char str[0x4000];	// 4KB
    _in->getline (str, 0x4000);
    if (_in->eof ())
    {
        //std::clog << ";; read: eof\n";
        obj = nullptr;
        throw std::runtime_error ("End_of_file");
        return (*this);
    }
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
jsonimstream::read (nlohmann::json& obj)
{
    //std::clog << ";; read\n";

    while (_messages.empty ())
    {
        //std::clog << ";; sleep\n";
        usleep (100000);	// 100ms
    }
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
    //std::clog << ";; write: '" << str << "'" << std::endl;
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
    assert (_mosq && _topic);

    std::string str = obj.dump ();
    const char* msg = str.c_str ();
    int len = str.length ();
    int rslt = mosquitto_publish (_mosq, nullptr, _topic, len, msg, 0, false);
    assert (rslt == MOSQ_ERR_SUCCESS);

    return (*this);
}
