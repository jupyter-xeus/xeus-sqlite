/***************************************************************************
* Copyright (c) 2020, Mariana Meireles                                     *
* Copyright (c) 2020, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <iostream>
#include <vector>
#include <sstream>
#include <stack>
#include <cctype>

#include "xeus/xinterpreter.hpp"

#include "xeus-sqlite/xeus_sqlite_interpreter.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

namespace xeus_sqlite
{

    std::string parse_code(const std::string& code)
    {
        //verifys if user wants to create a new db
        //create a map with the name of the db user is either creating or opening
        //do operations in this map of dbs
    }

    void interpreter::configure_impl()
    {
    }

    nl::json interpreter::execute_request_impl(int execution_counter,
                                               const std::string& code,
                                               bool /*silent*/,
                                               bool /*store_history*/,
                                               nl::json /*user_expressions*/,
                                               bool /*allow_stdin*/)
    {
        nl::json pub_data;
        std::string result = "";
        std::string path_to_db = "/Users/mariana/Development/jupyter-ecosystem/xeus-sqlite/chinook.db";
        SQLite::Database db(path_to_db, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);

        //if user wants to open a db or some other specific case
        // parse_code(code);

        //else execute normal sqlite code
        SQLite::Statement query(db, code);
        std::stringstream query_result("");

        //Iterates through the columns and prints the columns
        while (query.executeStep())
        {
            for(int column = 0; column < query.getColumnCount(); column++) {
                std::string col_content = query.getColumn(column);
                query_result << col_content << std::endl;
            }
        }

        query.reset();

        result += query_result.str();

        auto publish = [this](const std::string& name, const std::string& text) {
            this->publish_stream(name, text);
        };
        try
        {
            pub_data["text/plain"] = result;
            publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());
            nl::json jresult;
            jresult["status"] = "ok";
            jresult["payload"] = nl::json::array();
            jresult["user_expressions"] = nl::json::object();
            return jresult;
        }
        catch (const std::runtime_error& err)
        {
            nl::json jresult;
            publish_stream("stderr", err.what());
            jresult["status"] = "error";
            return jresult;
        }
    }

    nl::json interpreter::complete_request_impl(const std::string& /*code*/, int /*cursor_pos*/)
    {
        nl::json jresult;
        jresult["status"] = "ok";
        return jresult;
    };

    nl::json interpreter::inspect_request_impl(const std::string& /*code*/,
                                               int /*cursor_pos*/,
                                               int /*detail_level*/)
    {
        nl::json jresult;
        jresult["status"] = "ok";
        return jresult;
    };

    nl::json interpreter::is_complete_request_impl(const std::string& /*code*/)
    {
        nl::json jresult;
        jresult["status"] = "complete";
        return jresult;
    };

    nl::json interpreter::kernel_info_request_impl()
    {
        nl::json result;
        result["implementation"] = "xsqlite";
        result["implementation_version"] = "0.1.0";
        std::string banner = "";
        result["banner"] = banner;
        result["language_info"]["name"] = "sqlite";
        result["language_info"]["version"] = "";
        result["language_info"]["mimetype"] = "";
        result["language_info"]["file_extension"] = "";
        return result;
    }

    void interpreter::shutdown_request_impl()
    {
    }
}
