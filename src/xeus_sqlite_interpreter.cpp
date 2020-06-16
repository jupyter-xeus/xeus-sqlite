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
        Loads the database. If the open mode is not specified it defaults
        to read and write mode.
    */

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
        throw std::runtime_error("Wasn't able to load the database correctly.");
    }
}

void interpreter::create_db(const std::vector<std::string> tokenized_code)
{
    m_bd_is_loaded = true;
    m_db_path = tokenized_code[2];

    // Create the file
    std::ofstream(m_db_path.c_str()).close();

    // Create the database
    m_db = std::make_unique<SQLite::Database>(m_db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
}

void interpreter::delete_db()
{
    /*
        Deletes the database.
    */

    if(std::remove(m_db_path.c_str()) != 0)
    {
        throw std::runtime_error("Error deleting file.");
    }
}

nl::json interpreter::table_exists(const std::string table_name)
{
    nl::json pub_data;
    if (m_db->SQLite::Database::tableExists(table_name.c_str()))
    {
        pub_data["text/plain"] = "The table " + table_name + " exists.";
        return pub_data;
    }
    else
    {
        pub_data["text/plain"] = "The table " + table_name + " doesn't exist.";
        return pub_data;
    }
}

nl::json interpreter::is_unencrypted()
{
    nl::json pub_data;
    if (SQLite::Database::isUnencrypted(m_db_path))
    {
        pub_data["text/plain"] = "The database is unencrypted.";
        return pub_data;
    }
    else
    {
        pub_data["text/plain"] = "The database is encrypted.";
        return pub_data;
    }
}

nl::json interpreter::get_header_info()
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
    return pub_data;
}

void interpreter::backup(std::string backup_type)
{
    if (backup_type.size() > 1 && (int)backup_type[0] <= 1)
    {
        throw std::runtime_error("This is not a valid backup type.");
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
            throw std::runtime_error("The path doesn't exist.");
        }
        else
        {
            return load_db(tokenized_code);
        }
    }

    else if (tokenized_code[1] == "CREATE")
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
                publish_execution_result(execution_counter,
                    std::move(table_exists(tokenized_code[2])),
                    nl::json::object());
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
                publish_execution_result(execution_counter,
                    std::move(is_unencrypted()),
                    nl::json::object());
            }
            else if (tokenized_code[1] == "GET_INFO")
            {
                publish_execution_result(execution_counter,
                    std::move(get_header_info()),
                    nl::json::object());
            }
            else if (tokenized_code[1] == "BACKUP")
            {
                backup(tokenized_code[2]);
            }
        }
    else
    {
        throw std::runtime_error("Load a database to run this command.");
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
    std::vector<std::string> traceback;
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
            nl::json pub_data;

            tabulate::Table plain_table;
            std::stringstream html_table("");

            SQLite::Statement query(*m_db, code);

            if (query.getColumnCount() != 0)
            {
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
                html_table << "</table>";

                pub_data["text/plain"] = plain_table.str();
                pub_data["text/html"] = html_table.str();
                publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());
            }
            else
            {
                query.exec();
            }
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
        traceback.push_back((std::string)jresult["ename"] + ": " + (std::string)err.what());
        publish_execution_error(jresult["ename"], jresult["evalue"], traceback);
        traceback.clear();
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

    /* The jupyter-console banner for xeus-sqlite is the following:
      __  _____ _   _ ___
      \ \/ / _ \ | | / __|
       >  <  __/ |_| \__ \
      /_/\_\___|\__,_|___/
      xeus-sqlite: a Jupyter kernel for SQLite
    */

    std::string banner = ""
          "  __  _____ _   _ ___\n"
          "  \\ \\/ / _ \\ | | / __|\n"
          "   >  <  __/ |_| \\__ \\\n"
          "  /_/\\_\\___|\\__,_|___/\n"
          "\n"
          "  xeus-sqlite: a Jupyter kernel for SQLite\n"
          "  SQLite ";
    banner.append(SQLite::VERSION);

    result["banner"] = banner;
    result["language_info"]["name"] = "sqlite3";
    result["language_info"]["codemirror_mode"] = "sql";
    result["language_info"]["version"] = SQLite::VERSION;
    result["language_info"]["mimetype"] = "";
    result["language_info"]["file_extension"] = "";
    return result;
}

void interpreter::shutdown_request_impl()
{
}

}
