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
#include <memory>
#include <stack>
#include <cctype>

#include "xeus/xinterpreter.hpp"

#include "xeus-sqlite/xeus_sqlite_interpreter.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

namespace xeus_sqlite
{

std::vector<std::string> interpreter::tokenizer(const std::string& code)
{
    //TODO: this needs to be more rubust. I need to sanitize the string better
    //removing spaces and new lines from the begginning of the string, cause
    //those are valid inputs in a jupyter notebook, everything else can fail
    std::stringstream input(code);
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
    if(tokenized_code[0] == "%") {
        tokenized_code[1].erase(0, 1);
        std::transform(tokenized_code[1].begin(), tokenized_code[1].end(),tokenized_code[1].begin(), ::toupper);
        return true;
    }
    else
    {
        return false;
    }
}

void interpreter::load_db(std::vector<std::string> tokenized_code)
{
    /*
        Loads the a database. If the open mode is not specified it defaults
        to read and write mode.
    */

    try
    {
        if (tokenized_code[2] == tokenized_code.back())
        {
            this->db = std::make_unique<SQLite::Database>(tokenized_code[2],
                        SQLite::OPEN_READWRITE);
        }
        else if (tokenized_code.back().find("rw") != std::string::npos)
        {
            this->db = std::make_unique<SQLite::Database>(tokenized_code[2],
                        SQLite::OPEN_READWRITE);
        }
        else if (tokenized_code.back().find("r") != std::string::npos)
        {
            this->db = std::make_unique<SQLite::Database>(tokenized_code[2],
                        SQLite::OPEN_READONLY);
        }
        //TODO: treat the case where the user doesn't input anything
    }
    catch(const std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void interpreter::create_db(std::vector<std::string> tokenized_code)
{
    /*
        Creates the a database in read and write mode.
    */

    try
    {
        this->db = std::make_unique<SQLite::Database>(tokenized_code[2], SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
    }
    catch(const std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void interpreter::parse_code(std::vector<std::string> tokenized_code)
{
    //TODO: maybe add a rename? https://sqlite.org/lang_altertable.html#altertabrename

    std::cout << "You're using magic. " << std::endl;

    if (tokenized_code[1] == "LOAD")
    {
        load_db(tokenized_code);
    }
    else if (tokenized_code[1] == "CREATE")
    {
        create_db(tokenized_code);
    }
    //TODO: implement delete
    else if (tokenized_code[1] == "DELETE")
    {
    }
    //TODO: implement attach https://www.tutorialspoint.com/sqlite/sqlite_attach_database.htm
    else if (tokenized_code[1] == "ATTACH")
    {
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
    nl::json pub_data;
    std::string result = "";
    std::vector<std::string> tokenized_code = tokenizer(code);

    //Runs non-SQLite code
    if(is_magic(tokenized_code))
    {
        parse_code(tokenized_code);
    }
    else
    {
        SQLite::Statement query(*this->db, code);
        std::stringstream query_result("");

        //Iterates through the columns and prints them
        while (query.executeStep())
        {
            for(int column = 0; column < query.getColumnCount(); column++) {
                std::string col_content = query.getColumn(column);
                query_result << col_content << std::endl;
            }
        }
        query.reset();
        result += query_result.str();
    }

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
    // catch (const std::runtime_error& error)
    // {
        //TODO: deal with the error stuff
        // error = extract_error(error);

        // publish_execution_error(error.m_ename, error.m_evalue, error.m_traceback);
        // nl::json jresult;
        // jresult["status"] = "error";
        // jresult["ename"] = "Error";
        // jresult["evalue"] = error.what();
        // jresult["traceback"] = error.m_traceback.push_back(error.what());
        // return jresult;
    // }
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
