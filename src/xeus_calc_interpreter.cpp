/***************************************************************************
* Copyright (c) 2019, Sylvain Corlay, Johan Mabille, Wolf Vollprecht       *
* Copyright (c) 2019, QuantStack                                           *
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

#include "xeus-calc/xeus_calc_interpreter.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

using namespace std;

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
      {
        int i;
        cout << "Number of args= " << argc << endl;
       
        for(i=0; i<argc; i++)
        {
           cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << endl;
        }
        cout << endl;
        return 0;
      }


namespace xeus_calc
{
    void interpreter::configure_impl()
    {
    }

    std::string formating_expr(const std::string& expression)
    {
        std::string operators = "-+/*^()";
        std::string spaced_expression;
        std::string function;
        for (char itr : expression)
        {
            std::istringstream num(std::to_string(itr));
            size_t op = operators.find(itr);
            if (op != std::string::npos)
            {
                spaced_expression += ' ';
                spaced_expression += itr;
                spaced_expression += ' ';
            }
            else if (std::isdigit(itr) || std::isdigit(spaced_expression.back()) && itr == '.')
            {
                spaced_expression += itr;
            }
            // possible implementation for functions using the operators map defined below to check
            // for the function's existence
            /*else if (std::isalpha(itr))
            {

                function += itr;
                static operators_map_type operators_map = build_operators_map();
                auto it = operators_map.find(token);
                if (it != operators_map.end())
                {
                    function = "";
                }
            }*/
            else if (itr == ' ')
            {
                continue;
            }
            else
            {
                std::string s = "Syntax error :\none of the characters presents an issue : ";
                s.push_back(itr);
                throw std::runtime_error(s);
            }
        }
        return spaced_expression;
    }

    void howdoesthisworkfunction()
    {
      cout << "🦋🦋🦋" << endl;
    }

    nl::json interpreter::execute_request_impl(int execution_counter,
                                               const std::string& code,
                                               bool /*silent*/,
                                               bool /*store_history*/,
                                               nl::json /*user_expressions*/,
                                               bool /*allow_stdin*/)
    {
        nl::json pub_data;
        std::string result = "Result = ";
        string path_to_db = "/Users/mariana/Development/jupyter-ecosystem/xeus-sqlite/chinook.db";
        SQLite::Database    db(path_to_db, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        cout << db.getFilename().c_str() << endl;

        if (db.tableExists("test"))
        {
          cout << "🌈" << endl;
        }

          cout << "🌸" << endl;
          db.exec(code);
          // result += test;
        auto publish = [this](const std::string& name, const std::string& text) {
            this->publish_stream(name, text);
        };
        try
        {
            // std::string spaced_code = formating_expr(code);
            // result += std::to_string(compute_rpn(parse_rpn(spaced_code, publish), publish));
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
        // You can also use this method for publishing errors to the client, if the code
        // failed to execute
        // publish_execution_error(error_name, error_value, error_traceback);
        // publish_execution_error("TypeError", "123", {"!@#$", "*(*"});
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
        result["implementation"] = "xcalc";
        result["implementation_version"] = "0.1.0";
        std::string banner = ""
        " **     ** ******** **     **  ********         ******      **     **         ****** \n"
        "//**   ** /**///// /**    /** **//////         **////**    ****   /**        **////** \n"
        " //** **  /**      /**    /**/**              **    //    **//**  /**       **    // \n"
        "  //***   /******* /**    /**/********* *****/**         **  //** /**      /**       \n"
        "   **/**  /**////  /**    /**////////**///// /**        **********/**      /**       \n"
        "  ** //** /**      /**    /**       /**      //**    **/**//////**/**      //**    ** \n"
        " **   //**/********//*******  ********        //****** /**     /**/******** //****** \n"
        "//     // ////////  ///////  ////////          //////  //      // ////////   ////// \n"
        "\n"
        " Implementation of a calculator based on RPN through Xeus";
        result["banner"] = banner;
        result["language_info"]["name"] = "calc";
        result["language_info"]["version"] = "";
        result["language_info"]["mimetype"] = "";
        result["language_info"]["file_extension"] = "";
        return result;
    }

    void interpreter::shutdown_request_impl()
    {
    }
}
