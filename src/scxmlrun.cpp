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

#ifdef USE_QTSCXML
#include "adapters/qtscxmlproc.hpp"
#include <QtCore/QCoreApplication>
#define SCXMLPROC scxml::qtscxmlproc
#endif

#ifdef USE_USCXML
#include "adapters/uscxmlproc.hpp"
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
    std::cerr << "usage: " << prog << " <option>* <scxmlfile>? <infile>?\n";
    std::cerr << "options\n";
    std::cerr << "  -m <scxmlfile>\t"
              << "read scxml from <scxmlfile>\n";
    std::cerr << std::endl;

    std::cerr << "  -i <infile>\t\t"
              << "read input events from <infile>\n";
    std::cerr << "  -o <outfile>\t\t"
              << "write output events to <outfile>\n";
    std::cerr << "  --trace <tracefile>\t"
              << "write trace to <tracefile>\n";
    std::cerr << std::endl;

    std::cerr << "  --mqtt <host>\t\t"
              << "connect to <host> as the MQTT broker\n";
    std::cerr << "  --sub <topic>\t\t"
              << "subscribe to <topic> for input events\n";
    std::cerr << "  --pub <topic>\t\t"
              << "publish output events to <topic>\n";
    std::cerr << "  --trace-pub <topic>\t"
              << "publish trace to <topic>\n";
    std::cerr << std::endl;

    std::cerr << "  -h, --help\t\t"
              << "display this message\n";
    std::cerr << "  -v, --verbose\t\t"
              << "become verbose\n";
    std::cerr << "  -q, --silent\t\t"
              << "stay quiet\n";
    std::cerr << "  -V, --version\t\t"
              << "show version info\n";
}

static void
mosq_connect (mosquitto*& mosq, const char* host, scxml::scxmlproc* proc, bool& connected)
{
    if (mosq && connected) return;

    assert (!mosq);
    mosquitto_lib_init ();
    mosq = mosquitto_new (nullptr, true, nullptr);
    proc->mosq_set_callbacks (mosq);
    assert (mosq && !connected);

    //int rslt = mosquitto_connect (mosq, host, 1883, 60);
    int rslt = mosquitto_connect_async (mosq, host, 1883, 60);
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

    enum {_NONE = 0, _FILE, _MQTT} intype = _NONE, outtype = _NONE, tracetype = _NONE;
    const char* infile = NULL;
    const char* outfile = NULL;
    const char* tracefile = NULL;

    // in/out via MQTT
    const char* mqtt_host = "localhost";  // broker
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
        if (!strcmp (argv[i], "-m"))
            scxmlfile = argv[++i];

        else if (!strcmp (argv[i], "-i"))
            infile = argv[++i];
        else if (!strcmp (argv[i], "-o"))
            outfile = argv[++i];
        else if (!strcmp (argv[i], "--trace"))
            tracefile = argv[++i];

        else if (!strncmp (argv[i], "--mqtt", 6))
            mqtt_host = argv[++i];
        else if (!strncmp (argv[i], "--sub", 6))
            subs.push_back (argv[++i]);
        else if (!strncmp (argv[i], "--pub", 6))
            pub = argv[++i];

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
            synopsis (argv[0]); return 0;
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
        std::cerr << "** invalid input specification\n";
        return (-1);
    }
    if (infile)
        intype = _FILE;
    if (subs.size () > 0)
    {
        assert (intype == _NONE);
        assert (mqtt_host);
        intype = _MQTT;
    }
    if (intype == _NONE)
    {
        infile = "/dev/null";
        intype = _FILE;
    }

    // OUTPUT
    if (outfile && pub)
    {
        std::cerr << "** invalid output specification\n";
        return (-1);
    }
    if (outfile)
        outtype = _FILE;
    if (pub)
    {
        assert (outtype == _NONE);
        outtype = _MQTT;
    }
    if (outtype == _NONE)
    {
        outfile = (verbosity < 0) ? "/dev/null" : "/dev/stdout";
        outtype = _FILE;
    }

    // TRACE
    if (tracefile && trace_pub)
    {
        std::cerr << "** invalid trace specification\n";
        return (-1);
    }
    if (tracefile)
        tracetype = _FILE;
    if (trace_pub)
    {
        assert (tracetype == _NONE);
        assert (mqtt_host);
        tracetype = _MQTT;
    }
    if (tracetype == _NONE)
    {
        tracefile = (verbosity <= 0) ? "/dev/null" : "/dev/stderr";
        tracetype = _FILE;
    }

    // instantiate proc and load scxml
#ifdef USE_QTSCXML
    QCoreApplication app (argc, argv);	// non-GUI Qt application
    SCXMLPROC proc (&app);
#elif defined (USE_USCXML)
    SCXMLPROC proc;
#else
#error "scxml processor is unknown"
#endif
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
    proc.verbosity_set (verbosity);

    // load scxml
    assert (scxmlfile);
    proc.load (scxmlfile);

    // INPUT
    assert (intype != _NONE);
    switch (intype)
    {
    case _FILE:
        proc.eventin_open (infile);
        break;
    case _MQTT:
        {
            mosq_connect (mosq, mqtt_host, &proc, mosq_connected);
            proc.eventin_open (mosq, subs);
            mosq_start (mosq, mosq_started);

            /*
            if (!mosq)
            {
                mosq = mosquitto_new (nullptr, true, nullptr);
                proc.mosq_set_callbacks (mosq);
            }
            assert (mosq);

            if (!mosq_connected)
            {
                int rslt = mosquitto_connect (mosq, mqtt_host, 1883, 60);
                assert (rslt == MOSQ_ERR_SUCCESS);
                mosq_connected = true;
            }
            assert (mosq_connected);

            proc.eventin_open (mosq, subs);
            if (!mosq_started)
            {
                int rslt = mosquitto_loop_start (mosq);  // threaded
                assert (rslt == MOSQ_ERR_SUCCESS);
                mosq_started = true;
            }
            proc.eventin_open (mosq, subs);
            */
        }
        break;
    default:
        assert (false);
        return (-1);
    }

    // OUTPUT
    assert (outtype != _NONE);
    switch (outtype)
    {
    case _FILE:
        proc.eventout_open (outfile);
        break;
    case _MQTT:
        {
            mosq_connect (mosq, mqtt_host, &proc, mosq_connected);
            proc.eventout_open (mosq, pub);
            mosq_start (mosq, mosq_started);
        }
        break;
    default:
        assert (false);
        return (-1);
    }

    // TRACE
    switch (tracetype)
    {
    case _FILE:
        proc.traceout_open (tracefile);
        break;
    case _MQTT:
        {
            mosq_connect (mosq, mqtt_host, &proc, mosq_connected);
            proc.traceout_open (mosq, trace_pub);
            mosq_start (mosq, mosq_started);
        }
        break;
    default:
        assert (false);
        return (-1);
    }

    proc.setup ();
    int rslt = 0;
    try
    {
        rslt = proc.run ();
    }
    catch (const std::exception& e)
    {
        std::cerr << "** exception: " << e.what () << std::endl;
        rslt = -1;
    }
    mosq_terminate (mosq);

    return (rslt);

}
