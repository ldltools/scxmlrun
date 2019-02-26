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
#include "mqttrepeater.hpp"
#include "version.hpp"

#ifdef USE_QTSCXML
#include "qtscxml/qtscxmlproc.hpp"
#include <QtCore/QCoreApplication>
#define SCXMLPROC scxml::qtscxmlproc
#endif

#ifdef USE_USCXML
#include "uscxml/uscxmlproc.hpp"
#define SCXMLPROC scxml::uscxmlproc
#endif

#include <cstdio>
#include <codecvt>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <cassert>
//#include <thread>

static void
synopsis (const char* prog)
{
    std::cout << "usage: " << prog << " <option>* <scxmlfile>? <infile>?\n";
    std::cout << "options\n";
    std::cout << "  -m <scxmlfile>\t"
              << "read scxml from <scxmlfile>\n";
    std::cout << std::endl;

    std::cout << "  -i <infile>\t\t"
              << "read input events from <infile>\n";
    std::cout << "  -o <outfile>\t\t"
              << "write output events to <outfile>\n";
    std::cout << "  --trace <tracefile>\t"
              << "write trace to <tracefile>\n";
    std::cout << std::endl;

    std::cout << "  --mqtt\t\t"
              << "assume MQTT-based I/O by default\n";
    std::cout << "  --broker <host>\t"
              << "connect to <host> as the MQTT broker\n";
    std::cout << "  --sub <topic>\t\t"
              << "subscribe to <topic> for input events\n";
    std::cout << "  --pub <topic>\t\t"
              << "publish output events to <topic> by default\n";
    std::cout << "  --trace-pub <topic>\t"
              << "publish trace to <topic>\n";
    std::cout << std::endl;

    std::cout << "  -h, --help\t\t"
              << "display this message\n";
    std::cout << "  -v, --verbose\t\t"
              << "become verbose\n";
    std::cout << "  -q, --silent\t\t"
              << "stay quiet\n";
    std::cout << "  -V, --version\t\t"
              << "show version info\n";
}

static void
extra_synopsis (void)
{
    std::cout << std::endl;
    std::cout << "  [advanced/experimental options]" << std::endl;
    std::cout << "  -r, --relay\t\t"
              << "run as a MQTT event repeater\n";
}

static void
mosq_connect (mosquitto*& mosq, const char* host, int port, scxml::interpreter* proc, bool& connected)
{
    if (mosq && connected) return;

    assert (!mosq);
    mosquitto_lib_init ();
    mosq = mosquitto_new (nullptr, true, nullptr);
    proc->mosq_set_callbacks (mosq);
    assert (mosq && !connected);

    //int rslt = mosquitto_connect (mosq, host, 1883, 60);
    int rslt = mosquitto_connect_async (mosq, host, port, 60);
    assert (rslt == MOSQ_ERR_SUCCESS);
    connected = true;
}

static void
mosq_start (mosquitto* mosq, bool& started)
{
    assert (mosq);
    if (started) return;
    
    int rslt = mosquitto_loop_start (mosq);  // threaded
    assert (rslt == MOSQ_ERR_SUCCESS);
    started = true;
}

static void
mosq_terminate (mosquitto* mosq)
{
    if (!mosq) return;

    mosquitto_disconnect (mosq);
    mosquitto_loop_stop (mosq, false);
    mosquitto_destroy (mosq);
    mosquitto_lib_cleanup ();
}

int
main (int argc, char** argv)
{
    const char* scxmlfile = NULL;

    enum {_INTERPRETER, _REPEATER} mode = _INTERPRETER;
    enum {_NONE = 0, _FILE, _MQTT} intype = _NONE, outtype = _NONE, tracetype = _NONE;
    const char* infile = NULL;
    const char* outfile = NULL;
    const char* tracefile = NULL;

    // in/out via MQTT
    const char* mqtt_host = "localhost";  // broker
    int mqtt_port = 1883;
    std::list<const char*> subs;
    const char* pub = NULL;
    const char* trace_pub = NULL;

    int8_t verbosity = 0;
      // -1: silent
      //  0: default
      //  1: verbose
      //  2: extra-verbose

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp (argv[i], "-m") || !strncmp (argv[i], "--model", 5))
            scxmlfile = argv[++i];

        else if (!strcmp (argv[i], "-i"))
            infile = argv[++i];
        else if (!strcmp (argv[i], "-o"))
            outfile = argv[++i];
        else if (!strcmp (argv[i], "--trace"))
            tracefile = argv[++i];

        else if (!strncmp (argv[i], "--mqtt", 6))
            intype = outtype = tracetype = _MQTT; // this may be changed later
        else if (!strcmp (argv[i], "-b") || !strncmp (argv[i], "--broker", 5))
        {
            mqtt_host = argv[++i];
            const char* found = strchr (mqtt_host, ':');
            // case: <host>:<port>
            if (found)
            {
                sscanf (found, ":%d", &mqtt_port);
                assert (1000 < mqtt_port && mqtt_port < 9999);
                //*found = '\0';
                char* host = (char*) malloc (found - mqtt_host + 1);
                strncpy (host, mqtt_host, found - mqtt_host);
                host[found - mqtt_host] = '\0';
                mqtt_host = host;
            }
        }
        else if (!strncmp (argv[i], "--sub", 5))
            subs.push_back (argv[++i]);
        else if (!strncmp (argv[i], "--pub", 5))
            pub = argv[++i];

        else if (!strcmp (argv[i], "-r") || !strncmp (argv[i], "--relay", 5))
        {
            mode = _REPEATER;
            scxmlfile = "/dev/null";
        }

        else if (!strcmp (argv[i], "-v") || !strcmp (argv[i], "--verbose"))
            verbosity = 1;
        else if (!strcmp (argv[i], "-vv"))
            verbosity = 2;
        else if (!strcmp (argv[i], "-q") || !strcmp (argv[i], "--silent"))
            verbosity = -1;
        else if (!strcmp (argv[i], "-V") || !strcmp (argv[i], "--version"))
        {
            SCXMLPROC::version ();
            return 0;
        }
        else if (!strcmp (argv[i], "-h") || !strcmp (argv[i], "--help"))
        {
            std::cout << "scxmlrun v" << SCXMLRUN_VERSION << std::endl;
            synopsis (argv[0]);
            return 0;
        }
        else if (!strcmp (argv[i], "-hh"))
        {
            std::cout << "scxmlrun v" << SCXMLRUN_VERSION << std::endl;
            synopsis (argv[0]); extra_synopsis ();
            return 0;
        }

        else if (*argv[i] == '-')
        {
            std::cerr << "** unknown option: " << argv[i] << std::endl;
            return (1);
        }
        else
            if (!scxmlfile)
                scxmlfile = argv[i];
            else
                infile = argv[i];
    }

    if (!scxmlfile) { synopsis (argv[0]); return (0); }

    // INPUT
    if (infile && subs.size () > 0)
    {
        // more than one input sources: file and mqtt
        std::cerr << "** invalid input specification\n";
        return (-1);
    }
    if (!infile && subs.size () == 0)
        // fall-back
        if (intype != _MQTT)
            infile = "/dev/stdin";
        else
            subs.push_back ("scxmlrun");
    if (infile)
        intype = _FILE;
    else if (subs.size () > 0)
    {
        assert (intype != _FILE);
        assert (mqtt_host);
        intype = _MQTT;
    }

    assert (intype != _NONE);

    // OUTPUT
    if (outfile && pub)
    {
        // more than one output targets: file and mqtt
        std::cerr << "** invalid output specification\n";
        return (-1);
    }
    if (!outfile && !pub)
        // fall-back
        if (outtype != _MQTT)
            outfile = (verbosity < 0) ? "/dev/null" : "/dev/stdout";
        else
            pub = "scxmlrun";
    if (outfile)
        outtype = _FILE;
    if (pub)
        outtype = _MQTT;

    assert (outtype != _NONE);

    // TRACE
    if (tracefile && trace_pub)
    {
        std::cerr << "** invalid trace specification\n";
        return (-1);
    }
    if (!tracefile && !trace_pub)
        // fall-back
        if (tracetype != _MQTT)
            tracefile = (verbosity <= 0) ? "/dev/null" : "/dev/stderr";
        else
            trace_pub = "scxmlrun/trace";
    if (tracefile)
        tracetype = _FILE;
    if (trace_pub)
    {
        assert (mqtt_host);
        tracetype = _MQTT;
    }

    assert (tracetype != _NONE);

    // instantiate proc and load scxml
    scxml::interpreter* proc = nullptr;
    if (mode == _INTERPRETER)
    {
#ifdef USE_QTSCXML
        QCoreApplication* app = new QCoreApplication (argc, argv);
        // non-GUI Qt application
        proc = new SCXMLPROC (app);
#elif defined (USE_USCXML)
        proc = new SCXMLPROC ();
#else
#error "scxml processor is unknown"
#endif
    }
    else if (mode == _REPEATER)
    {
        assert (scxmlfile == "/dev/null");
        proc = new scxml::mqttrepeater ();
    }
    assert (proc);

    // MQTT broker
    mosquitto* mosq = nullptr;
    bool mosq_connected = false, mosq_started =false;

    // verbosity
    switch (verbosity)
    {
    case -1:
        std::cout.setstate (std::ios_base::badbit);
        std::cerr.setstate (std::ios_base::badbit);
    case 0:
        std::clog.setstate (std::ios_base::badbit);
        break;
    default:
        {}
    }
    proc->verbosity_set (verbosity);

    // INPUT
    assert (intype != _NONE);
    switch (intype)
    {
    case _FILE:
        proc->eventin_open (infile);
        break;
    case _MQTT:
        {
            mosq_connect (mosq, mqtt_host, mqtt_port, proc, mosq_connected);
            proc->eventin_open (mosq, subs);
            mosq_start (mosq, mosq_started);
        }
        break;
    default:
        assert (false);
        std::cerr << "** input type unspecified\n";
        return (-1);
    }

    // OUTPUT
    assert (outtype != _NONE);
    switch (outtype)
    {
    case _FILE:
        proc->eventout_open (outfile);
        break;
    case _MQTT:
        {
            mosq_connect (mosq, mqtt_host, mqtt_port, proc, mosq_connected);
            proc->eventout_open (mosq, pub);
            mosq_start (mosq, mosq_started);
        }
        break;
    default:
        assert (false);
        std::cerr << "** output type unspecified\n";
        return (-1);
    }

    // TRACE
    switch (tracetype)
    {
    case _FILE:
        proc->traceout_open (tracefile);
        break;
    case _MQTT:
        {
            mosq_connect (mosq, mqtt_host, mqtt_port, proc, mosq_connected);
            proc->traceout_open (mosq, trace_pub);
            mosq_start (mosq, mosq_started);
        }
        break;
    default:
        assert (false);
        std::cerr << "** trace type unspecified\n";
        return (-1);
    }

    // load scxml
    assert (scxmlfile);
    proc->load (scxmlfile);

    // init the engine, etc
    proc->setup ();

    // start processing
    int rslt = 0;
    try
    {
        rslt = proc->run ();
    }
    catch (const std::exception& e)
    {
        std::cerr << "** exception: " << e.what () << std::endl;
        rslt = -1;
    }
    delete proc;
    mosq_terminate (mosq);

    return (rslt);

}
