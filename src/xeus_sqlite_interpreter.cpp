/***************************************************************************
* Copyright (c) 2020, Mariana Meireles                                     *
* Copyright (c) 2020, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <cctype>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <stack>
#include <vector>

#include "xeus/xinterpreter.hpp"
#include "tabulate/table.hpp"

#include "xeus-sqlite/xeus_sqlite_interpreter.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

namespace xeus_sqlite
{

std::string sanitize_string(const std::string& code)
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
            m_bd_is_loaded = true;
            m_db = std::make_unique<SQLite::Database>(m_db_path,
                        SQLite::OPEN_READWRITE);
        }
        else if (tokenized_code.back().find("r") != std::string::npos)
        {
            m_bd_is_loaded = true;
            m_db = std::make_unique<SQLite::Database>(m_db_path,
                        SQLite::OPEN_READONLY);
        }
        //Opening as read and write because mode is unspecified
        else if (tokenized_code.size() < 4)
        {
            m_bd_is_loaded = true;
            m_db = std::make_unique<SQLite::Database>(m_db_path,
                        SQLite::OPEN_READWRITE);
        }
        else
        {
            std::string error = "Wasn't able to load the database correctly.";
            m_traceback.push_back("Error: " + error);
            publish_execution_error("Error", error, m_traceback);
            m_traceback.clear();
        }
    }
    catch (const std::runtime_error& err)
    {
        m_traceback.push_back("Error: " + (std::string)err.what());
        publish_execution_error("Error", err.what(), m_traceback);
        m_traceback.clear();
    }
}

void interpreter::create_db(const std::vector<std::string> tokenized_code)
{
    try
    {
        m_bd_is_loaded = true;
        m_db_path = tokenized_code[2];

        // Create the file
        std::ofstream(m_db_path.c_str()).close();

        // Create the database
        m_db = std::make_unique<SQLite::Database>(m_db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    }
    catch(const std::runtime_error& err)
    {
        m_traceback.push_back("Error: " + (std::string)err.what());
        publish_execution_error("Error", err.what(), m_traceback);
        m_traceback.clear();
    }
}

void interpreter::delete_db()
{
    /*
        Deletes the database.
    */

    if(std::remove(m_db_path.c_str()) != 0)
    {
        std::string error = "Error deleting file.";
        m_traceback.push_back("Error: " + error);
        publish_execution_error("Error", error, m_traceback);
        m_traceback.clear();
    }
}

void interpreter::table_exists(int execution_counter, const std::string table_name)
{
    if (m_db->SQLite::Database::tableExists(table_name.c_str()))
    {
        nl::json pub_data;
        pub_data["text/plain"] = "The table " + table_name + " exists.";
        publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());
    }
    else
    {
        nl::json pub_data;
        pub_data["text/plain"] = "The table " + table_name + " doesn't exist.";
        publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());
    }
}

void interpreter::is_unencrypted(int execution_counter)
{
    if (SQLite::Database::isUnencrypted(m_db_path))
    {
        nl::json pub_data;
        pub_data["text/plain"] = "The database is unencrypted.";
        publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());
    }
    else
    {
        nl::json pub_data;
        pub_data["text/plain"] = "The database is encrypted.";
        publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());
    }
}

void interpreter::get_header_info(int execution_counter)
{
    SQLite::Header header;
    header = SQLite::Database::getHeaderInfo(m_db_path);

   // Official documentation for fields can be found here: https://www.sqlite.org/fileformat.html#the_database_header
    nl::json pub_data;
    pub_data["text/plain"] =
        "Magic header string: " + std::string(&header.headerStr[0], &header.headerStr[15]) + "\n" +
        "Page size bytes: " + std::to_string(header.pageSizeBytes) + "\n" +
        "File format write version: " + std::to_string(header.fileFormatWriteVersion) + "\n" +
        "File format read version: " + std::to_string(header.fileFormatReadVersion) + "\n" +
        "Reserved space bytes: " + std::to_string(header.reservedSpaceBytes) + "\n" +
        "Max embedded payload fraction " + std::to_string(header.maxEmbeddedPayloadFrac) + "\n" +
        "Min embedded payload fraction: " + std::to_string(header.minEmbeddedPayloadFrac) + "\n" +
        "Leaf payload fraction: " + std::to_string(header.leafPayloadFrac) + "\n" +
        "File change counter: " + std::to_string(header.fileChangeCounter) + "\n" +
        "Database size pages: " + std::to_string(header.databaseSizePages) + "\n" +
        "First freelist trunk page: " + std::to_string(header.firstFreelistTrunkPage) + "\n" +
        "Total freelist trunk pages: " + std::to_string(header.totalFreelistPages) + "\n" +
        "Schema cookie: " + std::to_string(header.schemaCookie) + "\n" +
        "Schema format number: " + std::to_string(header.schemaFormatNumber) + "\n" +
        "Default page cache size bytes: " + std::to_string(header.defaultPageCacheSizeBytes) + "\n" +
        "Largest B tree page number: " + std::to_string(header.largestBTreePageNumber) + "\n" +
        "Database text encoding: " + std::to_string(header.databaseTextEncoding) + "\n" +
        "User version: " + std::to_string(header.userVersion) + "\n" +
        "Incremental vaccum mode: " + std::to_string(header.incrementalVaccumMode) + "\n" +
        "Application ID: " + std::to_string(header.applicationId) + "\n" +
        "Version valid for: " + std::to_string(header.versionValidFor) + "\n" +
        "SQLite version: " + std::to_string(header.sqliteVersion) + "\n";
    publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());
}

void interpreter::backup(std::string backup_type)
{
    if (backup_type.size() > 1 && (int)backup_type[0] <= 1)
    {
        std::string error = "This is not a valid backup type.";
        m_traceback.push_back("Error: " + error);
        publish_execution_error("Error", error, m_traceback);
        m_traceback.clear();
    }
    else
    {
        m_backup_db->SQLite::Database::backup(m_db_path.c_str(),
            (SQLite::Database::BackupType)backup_type[0]);
    }
}

void interpreter::parse_code(int execution_counter, const std::vector<std::string>& tokenized_code)
{
    if (tokenized_code[1] == "LOAD")
    {
        m_db_path = tokenized_code[2];

        std::ifstream path_is_valid(m_db_path);
        if (!path_is_valid.is_open())
        {
            std::string error = "The path doesn't exist.";
            m_traceback.push_back("Error: " + error);
            publish_execution_error("Error", error, m_traceback);
            m_traceback.clear();
        }
        else
        {
            return load_db(tokenized_code);
        }
    }

    if (tokenized_code[1] == "CREATE")
    {
        return create_db(tokenized_code);
    }

    if (m_bd_is_loaded)
    {
        if (tokenized_code[1] == "DELETE")
        {
            delete_db();
        }
        else if (tokenized_code[1] == "TABLE_EXISTS")
        {
            table_exists(execution_counter, tokenized_code[2]);
        }
        else if (tokenized_code[1] == "LOAD_EXTENSION")
        {
            //TODO: add a try catch to treat all void functions
            m_db->SQLite::Database::loadExtension(tokenized_code[2].c_str(),
                    tokenized_code[3].c_str());
        }
        else if (tokenized_code[1] == "SET_KEY")
        {
            m_db->SQLite::Database::key(tokenized_code[2]);
        }
        else if (tokenized_code[1] == "REKEY")
        {
            m_db->SQLite::Database::rekey(tokenized_code[2]);
        }
        else if (tokenized_code[1] == "IS_UNENCRYPTED")
        {
            is_unencrypted(execution_counter);
        }
        else if (tokenized_code[1] == "GET_INFO")
        {
            get_header_info(execution_counter);
        }
        else if (tokenized_code[1] == "BACKUP")
        {
            backup(tokenized_code[2]);
        }
    }
    else
    {
        std::string error = "Load a database to run this command.";
        m_traceback.push_back("Error: " + error);
        publish_execution_error("Error", error, m_traceback);
        m_traceback.clear();
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
    std::stringstream html_table("");
    std::vector<std::string> tokenized_code = tokenizer(code);

    try
    {
        //Runs non-SQLite code
        if(is_magic(tokenized_code))
        {
            parse_code(execution_counter, tokenized_code);
        }
        //Runs SQLite code
        else
        {
            SQLite::Statement query(*m_db, code);
            tabulate::Table plain_table;

            std::vector<std::variant<std::string, tabulate::Table>> column_names;
            html_table << "<table>\n<tr>\n";
            for (int column = 0; column < query.getColumnCount(); column++) {
                std::string name = query.getColumnName(column);
                column_names.push_back(name);
                html_table << "<th>" << name << "</th>\n";
            }
            plain_table.add_row(column_names);
            html_table << "</tr>\n";

            //Iterates through the columns and prints them
            while (query.executeStep())
            {
                html_table << "<tr>\n";
                std::vector<std::variant<std::string, tabulate::Table>> row;
                for (int column = 0; column < query.getColumnCount(); column++) {
                    std::string cell = query.getColumn(column);
                    row.push_back(cell);
                    html_table << "<td>" << cell << "</td>\n";
                }
                html_table << "</tr>\n";
                plain_table.add_row(row);
            }
            result += plain_table.str();
            html_table << "</table>";
        }

        if (result.size())
        {
            pub_data["text/plain"] = result;
            pub_data["text/html"] = html_table.str();
            publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());
        }

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
        m_traceback.push_back((std::string)jresult["ename"] + ": " + (std::string)err.what());
        publish_execution_error(jresult["ename"], jresult["evalue"], m_traceback);
        m_traceback.clear();
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
