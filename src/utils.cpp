#include "xeus-sqlite/utils.hpp"

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
        if(tokenized_input[0] == "XVEGA_PLOT")
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool is_magic(std::vector<std::string> tokenized_input)
    {
        /*
            Returns true if the code input is magic and false if isn't.
        */
        if(tokenized_input[0][0] == '%')
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    std::vector<std::string> tokenizer(const std::string& input)
    {
        /*
            Separetes the input with spaces.
        */
        std::stringstream sanitized_input(sanitize_string(input));
        std::string segment;
        std::vector<std::string> tokenized_input;

        while(std::getline(sanitized_input, segment, ' '))
        {
            tokenized_input.push_back(segment);
        }

        return tokenized_input;
    }

}