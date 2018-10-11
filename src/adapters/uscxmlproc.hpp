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

#ifndef USCXMLPROC_HPP
#define USCXMLPROC_HPP

#include "scxmlproc.hpp"
#include "uscxml/config.h"
#include "uscxml/uscxml.h"

#include <fstream>

namespace scxml {

class uscxmlproc : public scxmlproc
{
public:
    virtual void load (const std::string& scxml_url);
    virtual void setup (void);
    virtual int run (void);

public:
    uscxml::InterpreterState step (void);
    uscxml::InterpreterState state_get (void) { return (_interpreter.getState ()); }
    uscxml::Interpreter& interpreter (void) { return (_interpreter); }

public:
    void event_read (uscxml::Event&);
    void event_write (jsonostream&, const uscxml::Event&);

public:
    uscxmlproc (void);
    uscxmlproc (const uscxmlproc&);
    ~uscxmlproc (void);

public:
    virtual void verbosity_set (int);
    static void version (void);

private:
    void _common_init (void);
    void _init (void);
    void _datamodel_print (jsonostream&);

protected:
    uscxml::Interpreter _interpreter;
    uscxml::InterpreterMonitor* _trace_monitor;

private:
    static void json_to_data (nlohmann::json&, uscxml::Data&);
    static void data_to_json (uscxml::Data&, nlohmann::json&);

};

}

#endif
