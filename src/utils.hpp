/***************************************************************************
* Copyright (c) 2020, QuantStack and Xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SQLITE_UTILS_HPP
#define XEUS_SQLITE_UTILS_HPP

#include <string>
#include <vector>

namespace xeus_sqlite
{
    /**
     * Cleans the code from inputs that are acceptable in a jupyter notebook.
     */
    std::string sanitize_string(const std::string& code);

    bool case_insentive_equals(const std::string&, const std::string&);
    bool is_xvega(std::vector<std::string>& tokenized_input);
    bool is_magic(std::vector<std::string> tokenized_input);
    std::string to_lower(const std::string&);
    std::string to_upper(const std::string&);

    /**
     * Separates the code on spaces so it's easier to execute the commands.
     */
    std::vector<std::string> tokenizer(const std::string& code);
}

#endif
