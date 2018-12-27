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

#include "uscxmlproc.hpp"

#include "uscxml/uscxml.h"
#include "uscxml/interpreter/InterpreterMonitor.h"
#include "uscxml/interpreter/InterpreterImpl.h"
#include "uscxml/interpreter/ContentExecutor.h"
#include "uscxml/plugins/DataModel.h"
#include "uscxml/plugins/datamodel/ecmascript/JavaScriptCore/JSCDataModel.h"
#include "uscxml/util/DOM.h"

#include <JavaScriptCore/JavaScript.h>
#include <xercesc/dom/DOM.hpp>
//#include <xercesc/util/XercesVersion.hpp>
#include <boost/locale.hpp>

#include <cassert>
#include <codecvt>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

using namespace scxml;
using namespace XERCESC_NS;

// --------------------------------------------------------------------------------
// ctor/dtor
// --------------------------------------------------------------------------------

uscxmlproc::uscxmlproc (void) :
    scxml::interpreter (),
    _trace_monitor (nullptr)
{
}

uscxmlproc::uscxmlproc (const uscxmlproc& p) :
    scxml::interpreter ()
{
    assert (false);
}

uscxmlproc::~uscxmlproc (void)
{
    if (_trace_monitor) delete _trace_monitor;
}

void
uscxmlproc::_common_init (void)
{
}

void
uscxmlproc::load (const std::string& scxml_url)
{
    _interpreter = uscxml::Interpreter::fromURL (scxml_url);
    uscxml::Factory* factory = uscxml::Factory::getInstance ();
    assert (factory->hasDataModel ("ecmascript"));

    _init ();
}

void
uscxmlproc::_init (void)
{
    // scxml validation
    std::list<uscxml::InterpreterIssue> issues = _interpreter.validate ();
    for (std::list<uscxml::InterpreterIssue>::iterator iter = issues.begin (); iter != issues.end (); iter++)
    {
        std::cerr << "** " << *iter << std::endl;
    }

    // step forward until the interpreter becomes initialized
    std::cout.setstate (std::ios_base::badbit);
    while (_interpreter.getState () != uscxml::USCXML_INITIALIZED)
    {
        _interpreter.step ();
    }
    std::cout.clear ();

}

// --------------------------------------------------------------------------------
// eventin
// --------------------------------------------------------------------------------

void
uscxmlproc::event_read (uscxml::Event& e)
{
    nlohmann::json obj;
    _eventin->read (obj);

    if (obj.is_null ()) throw std::runtime_error ("Not_found");
    if (obj.find ("event") != obj.end ()) obj = obj["event"];
    //std::clog << obj << std::endl;

    uscxml::Data data;
    json_to_data (obj, data);
    e = uscxml::Event::fromData (data);
}

void
uscxmlproc::event_write (jsonostream&, const uscxml::Event&)
{
    assert (!"not yet implemented");
}

// --------------------------------------------------------------------------------
// traceout
// --------------------------------------------------------------------------------

// --------------------------------------------------------------------------------
// step
// --------------------------------------------------------------------------------

uscxml::InterpreterState
uscxmlproc::step (void)
{
    assert (_eventin);

    uscxml::InterpreterState state = _interpreter.getState ();
    switch (state)
    {
    case uscxml::USCXML_FINISHED:
        return (state);    

    case uscxml::USCXML_MACROSTEPPED:
        // A macrostep is a series of one or more microsteps ending in a configuration where the internal event queue is empty and no transitions are enabled by NULL.
        {
            //std::clog << _interpreter.serialize () << std::endl;
            //LOGD (USCXML_DEBUG) << interpreter.getImpl ().get ()->serialize () << std::endl;
            if (_traceout) _datamodel_print (*_traceout);

            uscxml::Event e;
            try { event_read (e); }
            catch (std::runtime_error ex)
            {
                if (strcmp (ex.what (), "End_of_file") == 0)
                    // events still may arrive later (delayed send)
                    assert (true);
                else
                    throw ex;
            }
            //std::clog << e << std::endl;
            _interpreter.receive (e);

            break;
        }
    default:
        {}
    }

    state = _interpreter.step ();
    const char* state_names[] = {"UNDEF", "IDLE", "INITIALIZED", "INSTANTIATED", "MICROSTEPPED", "MACROSTEPPED", "CANCELLED"};
    const char* state_name = state == uscxml::USCXML_FINISHED ? "FINISHED" : state_names[state];
    std::clog << ";; InterpreterState: " << state_name << std::endl;

    switch (state)
    {
    case uscxml::USCXML_UNDEF:
        throw std::runtime_error ("Not_found");
    case uscxml::USCXML_IDLE:
        return (step ());
    default:
        {}
    }

    return (state);
}

int
uscxmlproc::run (void)
{
    // run interpreter
    uscxml::InterpreterState state = state_get ();
    assert (state == uscxml::USCXML_INITIALIZED);
    while (state != uscxml::USCXML_FINISHED)
    {
        state = step ();
    }
    return (0);
}

// --------------------------------------------------------------------------------
// trace
// --------------------------------------------------------------------------------

static std::string
xmlch_to_string (const XMLCh* xmlchstr)
{
    std::wstring wstr = boost::locale::conv::utf_to_utf<wchar_t> (xmlchstr);
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
    return (cv.to_bytes (wstr));
}

static void
print_attributes (std::ostream& out, const DOMElement* elt)
{
    assert (elt);
    DOMNamedNodeMap* attrs = elt->getAttributes ();
    const XMLSize_t len = attrs->getLength ();
    for (XMLSize_t i = 0; i < len; i++)
    {
        DOMAttr* attr = (DOMAttr*) attrs->item (i);
        out << "\"" << xmlch_to_string (attr->getName ()) << "\""
            << " : "
            << "\"" << xmlch_to_string (attr->getValue ()) << "\"";
        if (i < len - 1) out << ", ";
    }
}

class TraceMonitor : public uscxml::InterpreterMonitor
{
public:
    // TransitionMonitor
    virtual void beforeEnteringState (const std::string& sessionId,
                                      const std::string& stateName,
                                      const DOMElement* state)
    {
        std::ostringstream out;
        out << "{\"state\" : {";
        print_attributes (out, state);
        out << "}}\n";
        nlohmann::json obj = nlohmann::json::parse (out.str ());
        assert (_traceout);
        _traceout->write (obj);
    }

    virtual void beforeTakingTransition (const std::string& sessionId,
                                         const DOMElement* transition)
    {
        std::ostringstream out;
        out << "{\"transition\" : {";
        print_attributes (out, transition);
        out << "}}\n";
        nlohmann::json obj = nlohmann::json::parse (out.str ());
        assert (_traceout);
        _traceout->write (obj);
    }

    virtual void beforeExitingState (const std::string& sessionId,
                                     const std::string& stateName,
                                     const DOMElement* state)
    {
        //*_stream << "{\"state\" : {";
        //print_attributes (*_stream, state);
        //*_stream << ", \"_direction\" : \"exiting\"";
        //*_stream << "}}\n";
    }

    //
    virtual void beforeProcessingEvent (const std::string& sessionId,
                                        const uscxml::Event& event)
    {
        std::ostringstream out;
        out << "{\"event\" : ";
        out << "{\"name\" : \"" << event.name << "\"";
        uscxml::Data data = event.data;
        if (!data.empty ())
        {
            out << ", \"data\" : ";
            out << data.asJSON ();
        }
        out << "}}\n";
        //std::clog << ";; event: " << out.str () << std::endl;

        nlohmann::json obj = nlohmann::json::parse (out.str ());
        assert (_traceout);
        _traceout->write (obj);
    }

    virtual void beforeExecutingContent (const std::string& sessionId,
                                         const XERCESC_NS::DOMElement* elt)
    {
        assert (elt->getNodeType () == 1);
        std::string tag = xmlch_to_string (elt->getTagName ());
        std::clog << ";; exec " << tag << ": {";
        print_attributes (std::clog, elt);
        std::clog << "}" << std::endl;

        if (tag.compare ("script") == 0)
        {
            assert (elt->hasChildNodes ());
            assert (elt->getChildElementCount () == 0);
            std::clog << ";; "
                      << xmlch_to_string (elt->getTextContent ())
                      << std::endl;
        }
    }

    virtual void beforeInvoking (const std::string& sessionId,
                                 const XERCESC_NS::DOMElement* elt,
                                 const std::string& invokeid)
    {
        std::string tag = xmlch_to_string (elt->getTagName ());
        std::clog << ";; exec " << tag << ": {";
        print_attributes (std::clog, elt);
        std::clog << "}" << std::endl;
    }

public:
    TraceMonitor (void) :
        _proc (nullptr),
        _traceout (nullptr)
    {
        std::clog << ";; monitor generated\n";
    }

private:
    scxml::interpreter* _proc;
    jsonostream* _traceout;
    //_JSCDataModel* _datamodel;

    friend class scxml::uscxmlproc;

};

// --------------------------------------------------------------------------------
// uscxml hack
// --------------------------------------------------------------------------------

class _InterpreterImpl : public uscxml::InterpreterImpl
{
public:
    uscxml::DataModel* getDataModel (void) { return (&_dataModel); }

    uscxml::Data
    _evalDataModel (void)
    {
        assert (_state == uscxml::USCXML_IDLE || _state == uscxml::USCXML_MACROSTEPPED || _state == uscxml::USCXML_FINISHED);

        std::lock_guard<std::recursive_mutex> lock (_serializationMutex);

        uscxml::Data data;
        //auto prefix = XML_PREFIX(_scxml);
        auto prefix = uscxml::X (_scxml->getPrefix () ? uscxml::X (_scxml->getPrefix ()).str() + ":" : "");
        std::list<XERCESC_NS::DOMElement*> data_elts = uscxml::DOMUtils::inDocumentOrder ({ prefix.str () + "data" }, _scxml);
        for (auto elt : data_elts) {
            if (elt->hasAttribute (uscxml::kXMLCharId))
            {
                //auto attr = ATTR (elt, uscxml::kXMLCharId);
                auto attr = std::string (uscxml::X (elt->getAttribute (uscxml::kXMLCharId)));
                data["datamodel"][attr] = _dataModel.evalAsData (attr);
            }
        }
        return (data);
    }
};

class _DataModel : public uscxml::DataModel
{
public:
    uscxml::DataModelImpl* getImpl () { return (_impl.get ()); }
};

class _JSCDataModel : public uscxml::JSCDataModel
{
public:
    //DataModelImpl* getImpl () { return (_impl.get ()); }
    JSGlobalContextRef getContext () { return (_ctx); }
};

void
uscxmlproc::_datamodel_print (jsonostream& out)
{
    _InterpreterImpl* impl = (_InterpreterImpl*) _interpreter.getImpl (). get ();
    assert (impl);
    uscxml::Data data = impl->_evalDataModel ();
    nlohmann::json obj;
    data_to_json (data, obj);
    if (obj == nullptr) return;

    out.write (obj);
}

// --------------------------------------------------------------------------------
// setup
// --------------------------------------------------------------------------------

// console.log
static JSValueRef
ConsoleLogCallback (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                    size_t argumentCount, const JSValueRef arguments[],
                    JSValueRef* exception)       
{
    assert (argumentCount >= 1);

    const JSValueRef value = arguments[0];

    JSStringRef str;
    switch (JSValueGetType (ctx, value))
    {
    case kJSTypeString:
        str = JSValueToStringCopy (ctx, value, exception);
        break;
    default:
        str = JSValueCreateJSONString (ctx, value, 0, exception);
    }

    char buff[100];
    JSStringGetUTF8CString (str, buff, 100);

    std::cout << buff << std::endl;

    return JSValueMakeUndefined (ctx);
}

void
uscxmlproc::verbosity_set (int v)
{
    switch (v)
    {
    case -1:
        cout ().setstate (std::ios_base::badbit);
        cerr ().setstate (std::ios_base::badbit);
        break;
    case 0:
        std::clog.setstate (std::ios_base::badbit);
        break;
    case 1:
        cout ().clear ();
        cerr ().clear ();
        std::clog.clear ();
        break;
    case 2:
        cout ().clear ();
        cerr ().clear ();
        std::clog.clear ();
        break;
    default:
        {}
    }
}

void
uscxmlproc::setup (void)
{
    // eventin
    assert (_eventin);

    // trace_monitor
    if (_traceout)
    {
        TraceMonitor* mon = new TraceMonitor ();
        mon->_proc = this;
        //mon->_datamodel = (_JSCDataModel*) dm_i;
        mon->_traceout = _traceout;

        _trace_monitor = mon;
        _interpreter.addMonitor (_trace_monitor);
    }

    // ** quick hack: define & intern "console.log"
    assert (_interpreter.getState () == uscxml::USCXML_INITIALIZED);

    //DataModelImpl* dm_i = ((XDataModel*)dm)->getImpl ();
    //assert (dm_i);
    uscxml::ActionLanguage* al = _interpreter.getActionLanguage ();
    uscxml::DataModel* dm = &al->dataModel;
    bool dm_is_ecmascript = false;
    for (auto name : dm->getNames ())
        if (name == "ecmascript") { dm_is_ecmascript = true; break; }
    if (!dm_is_ecmascript) return;

    uscxml::DataModelImpl* dm_i = ((_DataModel*)dm)->getImpl ();
    assert (dm_i);
    JSGlobalContextRef global_ctx = ((_JSCDataModel*)dm_i)->getContext ();
    assert (global_ctx);

    // bad raw opaque pointers TODO: manage with smart pointers 
    //JSContextGroupRef group = JSContextGroupCreate();
    //JSGlobalContextRef globalContext = JSGlobalContextCreateInGroup (group, nullptr);
    JSObjectRef global_obj = JSContextGetGlobalObject (global_ctx);

    // make function object
    JSStringRef consolePairsName = JSStringCreateWithUTF8CString ("console");
    JSObjectRef consolePairsObj = JSObjectMake (global_ctx, NULL, NULL);
    JSStringRef logFuncName = JSStringCreateWithUTF8CString ("log");
    JSObjectRef logFuncObj = JSObjectMakeFunctionWithCallback ( global_ctx,
                                                                logFuncName,
                                                                &ConsoleLogCallback);
    JSObjectSetProperty (global_ctx, consolePairsObj, logFuncName, logFuncObj,
                         kJSPropertyAttributeNone, nullptr );

    // set it as proprty of global object
    JSObjectSetProperty (global_ctx, global_obj,
                         consolePairsName, consolePairsObj,
                         kJSPropertyAttributeNone, nullptr);
}

void
uscxmlproc::version (void)
{
    std::cerr << "uSCXML:\t\t"
              << USCXML_VERSION
              << std::endl;

    std::cerr << "JSCore:\t\t"
              << "unknown (webkitgtk-4.0)"
              << std::endl;

    std::cerr << "XercesC:\t"
              << XERCES_FULLVERSIONSTR
              << std::endl;

    auto json_meta = nlohmann::json::meta ();
    std::cerr << "JSON for C++:\t"
              << json_meta["version"]["major"] << "."
              << json_meta["version"]["minor"] << "."
              << json_meta["version"]["patch"]
              << std::endl;

    std::cerr << "Mosquitto:\t"
              << LIBMOSQUITTO_MAJOR << "."
              << LIBMOSQUITTO_MINOR << "."
              << LIBMOSQUITTO_REVISION
              << std::endl;
}

// --------------------------------------------------------------------------------
// utils
// --------------------------------------------------------------------------------

void
uscxmlproc::json_to_data (nlohmann::json& obj, uscxml::Data& data)
{
    //assert (obj.is_object ());
    std::string str = obj.dump ();
    //auto name = obj["name"];
    data = uscxml::Data::fromJSON (str);
}

void
uscxmlproc::data_to_json (uscxml::Data& data, nlohmann::json& obj)
{
    //assert (obj.is_object ());
    std::string str = data.asJSON ();
    //auto name = obj["name"];
    obj = nlohmann::json::parse (str);
}
