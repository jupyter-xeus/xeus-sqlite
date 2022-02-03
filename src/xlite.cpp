/***************************************************************************
* Copyright (c) 2022, Thorsten Beier                                       *                                                       *
* Copyright (c) 2022, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/


// tools for the jupyterlite kernels
#ifdef XSQL_EMSCRIPTEN_WASM_BUILD

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>

#include <iostream>
#include <fstream>  
#include <sstream>

#include "xeus/xinterpreter.hpp"

// this might end up in a dedicated library at some point
namespace xeus_lite
{
    namespace detail
    {
        struct fetch_user_data
        {
            std::string filename;
            bool done;
        };
    }

    // download a file from url and store in fileystem
    // progress is reported to jupyters `stdout` stream 
    void fetch(const std::string url, const std::string filename)
    {
        std::stringstream ss;
        ss<<"Start downloading from URL "<<url<<"\n";
        auto & interpreter = xeus::get_interpreter();
        interpreter.publish_stream("stdout", ss.str());

        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;\
        auto  userData = new detail::fetch_user_data{
            std::string(filename),
            false
        };
        attr.userData = userData;

        attr.onsuccess = [](emscripten_fetch_t *fetch){
            auto & interpreter = xeus::get_interpreter();

            {
                std::stringstream s;
                s<<"Finished downloading "<<fetch->numBytes<<" bytes from URL "<<fetch->url<<"\n";
                interpreter.publish_stream("stdout", s.str());
            }

            auto userData = reinterpret_cast<fetch_user_data *>(fetch->userData);
            auto filename = userData->filename;
            std::ofstream myFile;
            {
                std::stringstream s;
                s<<"Writing downloaded data to file "<<filename<<"\n";
                interpreter.publish_stream("stdout", s.str());
            }

            myFile.open(filename, std::ios::out | std::ios::binary);
            myFile.write (fetch->data, fetch->numBytes);
            myFile.close();
            interpreter.publish_stream("stdout","Finished writing file\n");
            userData->done = true;
            emscripten_fetch_close(fetch);
        };
        attr.onerror = [](emscripten_fetch_t *fetch){
            std::stringstream s;
            s<<"Downloading "<<fetch->url<<"failed , HTTP failure status code:"<<fetch->status<<"\n";
            auto & interpreter = xeus::get_interpreter();
            std::cout<<s.str()<<"\n";
            interpreter.publish_stream("stdout", s.str());
            auto userData = reinterpret_cast<fetch_user_data *>(fetch->userData);
            userData->done = true;
            emscripten_fetch_close(fetch);
        };
        attr.onprogress = [](emscripten_fetch_t *fetch){
            std::stringstream s;
            auto & interpreter = xeus::get_interpreter();

            if (fetch->totalBytes) {
                s<<"Downloading "<<fetch->url<<" "<<std::setprecision(2)<<fetch->dataOffset * 100.0 / fetch->totalBytes<<"% complete\n";
            } else 
            {
                s<<"Downloading "<<fetch->url<<" "<<fetch->dataOffset + fetch->numBytes<<"bytes complete\n";
            }
            std::cout<<s.str()<<"\n";
            interpreter.publish_stream("stdout", s.str());
        };
        emscripten_fetch(&attr, url.c_str());

        while(!userData->done)
        {
            emscripten_sleep(100);
        }
        delete userData;
    }

    // make a directory an IDBFS filesystem
    EM_JS(void, async_ems_init_idbfs,  (const char* path), {
        return Asyncify.handleSleep(function (wakeUp) {
            var jpath = UTF8ToString(path);
            FS.mkdir( jpath); 
            FS.mount(IDBFS,{}, jpath);
            FS.syncfs(true, function(err) {
                assert(!err);
                wakeUp();
            });
        });
    });

    // write / flush files from in mem filesystem to IDB
    EM_JS(void, async_ems_sync_db,  (), {
        return Asyncify.handleSleep(function (wakeUp) {;
            FS.syncfs(false, function(err) {
                assert(!err);
                wakeUp();
            });
        });
    });

    // make a directory an IDBFS filesystem
    void ems_init_idbfs(const std::string & path)
    {
        async_ems_init_idbfs(path.c_str());
    }

    // write / flush files from in mem filesystem to IDB
    void ems_sync_db()
    {   
        async_ems_sync_db();
    }
}
#endif // XSQL_EMSCRIPTEN_WASM_BUILD