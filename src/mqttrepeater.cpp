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

#include "mqttrepeater.hpp"
#include <iostream>

using namespace scxml;

mqttrepeater::mqttrepeater (void)
{
}

mqttrepeater::~mqttrepeater (void)
{
}

void
mqttrepeater::load (const std::string&)
{
}

void
mqttrepeater::setup (void)
{
}

int
mqttrepeater::run (void)
{
    assert (_eventin && _eventout);

    while (1)
    {
        nlohmann::json obj;
        try { _eventin->read (obj); }
        catch (std::runtime_error ex)
        {
            if (!strcmp (ex.what (), "End_of_file")) break;
        }

        if (obj.find ("type") == obj.end ()) obj["type"] = "mqtt";

        std::clog << obj.dump ().c_str () << std::endl;
        _eventout->write (obj);
    }
}

void
mqttrepeater::verbosity_set (int)
{
}
