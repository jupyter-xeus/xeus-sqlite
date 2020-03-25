/***************************************************************************
* Copyright (c) 2020, Mariana Meireles                                     *
* Copyright (c) 2020, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SQLITE_INTERPRETER_HPP
#define XEUS_SQLITE_INTERPRETER_HPP

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

#include "xeus/xinterpreter.hpp"
#include "nlohmann/json.hpp"
#include "xeus_sqlite_config.hpp"

namespace nl = nlohmann;

namespace xeus_sqlite
{
class XEUS_SQLITE_API interpreter : public xeus::xinterpreter
{
    friend class SQLite::Database;

public:

    interpreter() = default;
    virtual ~interpreter() = default;

private:

    void configure_impl() override;
    nl::json execute_request_impl(int execution_counter,
                                  const std::string& code,
                                  bool silent,
                                  bool store_history,
                                  nl::json user_expressions,
                                  bool allow_stdin) override;
    nl::json complete_request_impl(const std::string& code,
                                   int cursor_pos) override;
    nl::json inspect_request_impl(const std::string& code,
                                  int cursor_pos,
                                  int detail_level) override;
    nl::json is_complete_request_impl(const std::string& code) override;
    nl::json kernel_info_request_impl() override;
    void shutdown_request_impl() override;

    std::string sanitize_string(const std::string& code);
    std::vector<std::string> tokenizer(const std::string& code);
    void parse_code(const std::vector<std::string>& tokenized_code);
    bool is_magic(std::vector<std::string>& tokenized_code);
    void load_db(const std::vector<std::string> tokenized_code);
    void create_db(const std::vector<std::string> tokenized_code);
    void delete_db();
    void table_exists(const std::string table_name);

    std::unique_ptr<SQLite::Database> m_db = nullptr;
    std::string m_db_path;
};

}

#endif
