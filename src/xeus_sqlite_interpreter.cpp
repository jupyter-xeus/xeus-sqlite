/***************************************************************************
* Copyright (c) 2020, Mariana Meireles                                     *
* Copyright (c) 2020, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <memory>
#include <stack>
#include <cctype>
#include <cstdio>

#include "xeus/xinterpreter.hpp"

#include "xeus-sqlite/xeus_sqlite_interpreter.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

namespace xeus_sqlite
{

std::string interpreter::sanitize_string(const std::string& code)
{
    /*
        Cleans the code from inputs that are acceptable in a jupyter notebook.
    */
    std::string aux = code;
    aux.erase(
        std::remove_if(
            aux.begin(), aux.end(), [](char const c) {
                return '\n' == c || '\r' == c || '\0' == c || '\x1A' == c;
            }
        ),
        aux.end()
    );
    return aux;
}

std::vector<std::string> interpreter::tokenizer(const std::string& code)
{
    /*
        Separetes the code on spaces so it's easier to execute the commands.
    */
    std::stringstream input(sanitize_string(code));
    std::string segment;
    std::vector<std::string> tokenized_str;
    std::string is_magic(1, input.str()[0]);
    tokenized_str.push_back(is_magic);

    while(std::getline(input, segment, ' '))
    {
        tokenized_str.push_back(segment);
    }


    return tokenized_str;
}

bool interpreter::is_magic(std::vector<std::string>& tokenized_code)
{
    /*
        Returns true if the code input is magic and false if isn't.
    */
    if(tokenized_code[0] == "%")
    {
        tokenized_code[1].erase(0, 1);
        std::transform(tokenized_code[1].begin(), tokenized_code[1].end(),
                        tokenized_code[1].begin(), ::toupper);
        return true;
    }
    else
    {
        return false;
    }
}

void interpreter::load_db(const std::vector<std::string> tokenized_code)
{
    /*
        Loads the a database. If the open mode is not specified it defaults
        to read and write mode.
    */

    try
    {
        if (tokenized_code.back().find("rw") != std::string::npos)
        {
            m_db = std::make_unique<SQLite::Database>(m_db_path,
                        SQLite::OPEN_READWRITE);
        }
        else if (tokenized_code.back().find("r") != std::string::npos)
        {
            m_db = std::make_unique<SQLite::Database>(m_db_path,
                        SQLite::OPEN_READONLY);
        }
        //Opening as read and write because mode is unspecified
        else if (tokenized_code.size() < 2)
        {
            m_db = std::make_unique<SQLite::Database>(m_db_path,
                        SQLite::OPEN_READWRITE);
        }
    }
    catch (const std::runtime_error& err)
    {
        nl::json jresult;
        jresult["status"] = "error";
        jresult["ename"] = "Error";
        jresult["evalue"] = err.what();
        publish_stream("stderr", err.what());
    }
}

void interpreter::create_db(const std::vector<std::string> tokenized_code)
{
    /*
        Creates the a database in read and write mode.
    */

    try
    {
        m_db_path = tokenized_code[2];
        m_db_path += tokenized_code[3];
        m_db = std::make_unique<SQLite::Database>(m_db_path,
                        SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
    }
    catch(const std::runtime_error& err)
    {
        nl::json jresult;
        jresult["status"] = "error";
        jresult["ename"] = "Error";
        jresult["evalue"] = err.what();
        publish_stream("stderr", err.what());
    }
}

void interpreter::delete_db()
{
    /*
        Deletes the database.
    */

    if(std::remove(m_db_path.c_str()) != 0)
    {
        publish_stream("stderr", "Error deleting file.\n");
    }
    else
    {
        publish_stream("stderr", "File successfully deleted.\n");
    }
}

void interpreter::parse_code(const std::vector<std::string>& tokenized_code)
{
    std::cout << "You're using magic. " << std::endl;

    m_db_path = tokenized_code[2];

    std::ifstream path_is_valid(m_db_path); 
    if (!path_is_valid)
    {
      publish_stream("stderr", "The path doesn't exist.\n");
    }
    else
    {
        if (tokenized_code[1] == "LOAD")
        {
            load_db(tokenized_code);
        }
        else if (tokenized_code[1] == "CREATE")
        {
            create_db(tokenized_code);
        }
        else if (tokenized_code[1] == "DELETE")
        {
            delete_db();
        }
    }
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
    /*
        Executes either SQLite code or Jupyter Magic.
    */

    nl::json pub_data;
    std::string result = "";
    std::vector<std::string> tokenized_code = tokenizer(code);

    try
    {
        //Runs non-SQLite code
        if(is_magic(tokenized_code))
        {
            parse_code(tokenized_code);
        }
        //Runs SQLite code
        else
        {
            SQLite::Statement query(*m_db, code);
            std::stringstream query_result("");

            //Iterates through the columns and prints them
            while (query.executeStep())
            {
                for(int column = 0; column < query.getColumnCount(); column++) {
                    std::string name = query.getColumn(column);
                    query_result << name << std::endl;
                }
            }
            query.reset();
            result += query_result.str();
        }

        pub_data["text/plain"] = result;
        publish_execution_result(execution_counter, std::move(pub_data),
                                    nl::json::object());
        nl::json jresult;
        jresult["status"] = "ok";
        jresult["payload"] = nl::json::array();
        jresult["user_expressions"] = nl::json::object();
        return jresult;
    }

    catch (const std::runtime_error& err)
    {
        nl::json jresult;
        jresult["status"] = "error";
        jresult["ename"] = "Error";
        jresult["evalue"] = err.what();
        publish_stream("stderr", err.what());
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
