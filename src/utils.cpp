/***************************************************************************
* Copyright (c) 2020, QuantStack and Xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "utils.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

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

    bool case_insentive_equals(const std::string& a, const std::string& b)
    {
        return std::equal(a.begin(), a.end(), b.begin(),
                    [](unsigned char a, unsigned char b) {
                          return std::tolower(a) == std::tolower(b);
                    });
    }

    bool is_xvega(std::vector<std::string>& tokenized_input)
    {
        /*
            Returns true if the code input is xvega and false if isn't.
        */
        return tokenized_input[0] == "XVEGA_PLOT";
    }

    bool is_magic(std::vector<std::string> tokenized_input)
    {
        /*
            Returns true if the code input is magic and false if isn't.
        */
        return tokenized_input[0][0] == '%';
    }

    std::vector<std::string> tokenizer(const std::string& input)
    {
        /*
            Separetes the input with spaces.
        */
        std::istringstream sanitized_input(sanitize_string(input));
        std::string segment;
        std::vector<std::string> tokenized_input;

        while(sanitized_input >> segment)
        {
            tokenized_input.push_back(segment);
        }

        return tokenized_input;
    }

    std::string to_lower(const std::string& input)
    {
        std::string lower_case_input;
        lower_case_input.resize(input.length());
        std::transform(input.begin(), input.end(), lower_case_input.begin(), ::tolower);
        return lower_case_input;
    }

    std::string to_upper(const std::string& input)
    {
        std::string upper_case_input;
        upper_case_input.resize(input.length());
        std::transform(input.begin(), input.end(), upper_case_input.begin(), ::toupper);
        return upper_case_input;
    }

}