#include "xeus-sqlite/utils.hpp"

#include <algorithm>
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

    void normalize_string(std::vector<std::string>& tokenized_input)
    {
        tokenized_input[1].erase(0, 1);
        std::transform(tokenized_input[1].begin(), tokenized_input[1].end(),
                        tokenized_input[1].begin(), ::toupper);
    }

    bool is_xvega(std::vector<std::string>& tokenized_input)
    {
        /*
            Returns true if the code input is xvega and false if isn't.
        */
        if(tokenized_input[1] == "XVEGA_PLOT")
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
        if(tokenized_input[0] == "%")
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    std::vector<std::string> tokenizer(const std::string& code)
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

}