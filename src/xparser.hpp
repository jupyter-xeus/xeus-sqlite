/***************************************************************************
* Copyright (c) 2020, QuantStack and Xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <map>

#include "utils.hpp"

namespace nl = nlohmann;

namespace xeus_sqlite
{
    /**
        Base parser class, should be inherited to define concrete parsers. This class
        contains helpers to call appropriate parsing functions based on a parsing map,
        which is a mapping of strings to `command_info` structures.

        Subclasses should initialize the parsing map pointing to its own methods, eg:

          struct concrete_parser : parser_base<concrete_parser>
          {
              concrete_parser()
              {
                  parsing_table = {
                      {"MY_TOKEN",    {1, &concrete_parser::parse_my_token}},
                      {"OTHER_TOKEN", {1, &concrete_parser::parse_other_token}},
                  };
              }
              
              void
              parse_my_token(const input_it& it)
              {
                  cout << "my token!" << endl;
              }
              
              input_it
              parse_other_token(const input_it& begin, const input_it& end)
              {
                  cout << "other token!" << endl;
                  return begin+1;
              }
          }

        The subclass can then be used to parse a stream of tokens:

          concrete_parser p;
          auto last_parsed = p.parse_loop(token_list.begin(), token_list.end());
    **/
    template<typename T>
    struct parser_base
    {
        using input_it = std::vector<std::string>::iterator;
        /**
            Parse functions are methods of child classes, can have one of two types:
            
             - point_it_fun: takes a single iterator, for parse functions that
                look at either one token ahead, or don't look at any tokens. The
                parsing iterator is always advanced by one position after a parse
                function of this type is called.
        **/
        using point_it_fun = std::function<void(T*, const input_it&)>;
        /**
             - range_it_function: takes a pair of iterators (begin, end) corresponding
                 to the current position of the parsing iterator and the end of the
                 stream of tokens. For parse functions that look at more than one
                 token ahead in the input stream. Should return an iterator in the
                 input stream corresponding to the position where parsing should
                 continue, eg. one after the last token that was successfully parsed
                 by this function.
        **/
        using range_it_fun = std::function<input_it(T*,
                                                    const input_it&,
                                                    const input_it&)>;
        using parse_function_types = xtl::variant<point_it_fun,
                                                  range_it_fun>;

        struct command_info {
            /**
                Minimum number of tokens that should exist in the stream after this
                command is seen.
            **/
            int number_required_arguments;
            /** Parsing function to call when this command is seen **/
            parse_function_types parse_function;
        };

        using free_fun = std::function<void()>;

        std::map<std::string, command_info> mapping_table;

        /** Switch implementation with strings **/
        bool simple_switch(const std::string& token,
                           std::map<std::string,
                           free_fun> handlers)
        {
            auto handler_it = handlers.find(to_upper(token));
            if (handler_it == handlers.end()) {
                return false;
            }
            /** Call the handler **/
            handler_it->second();
            return true;
        }

        /**
            Override this function to be able to parse any initial tokens before the
            parsing table takes over processing of tokens. This can be used for cases
            where a command has one or more required attributes before all optional
            attributes are handled.
        **/
        input_it parse_init(const input_it& begin, const input_it& end)
        {
            return begin;
        }

        /** Parse a single token (and its dependencies)
           Returns an iterator past the last successfully parsed token
        **/
        input_it parse_step(const input_it& begin, const input_it& end)
        {
            input_it it = begin;

            auto cmd_it = mapping_table.find(to_upper(*it));
            if (cmd_it == mapping_table.end())
            {
                return it; 
            }

            command_info cmd_info = cmd_it->second;

            /** Prevents code to end prematurely **/
            if (std::distance(it, end) < cmd_info.number_required_arguments)
            {
                throw std::runtime_error("Arguments missing.");
            }

            /** Advances to next token **/
            ++it;

            /** Calls parsing function for command **/
            if (cmd_info.parse_function.index() == 0)
            {
                /** Calls command functions that receive a point iterator **/
                xtl::get<0>(cmd_info.parse_function)(static_cast<T*>(this), it);
                it++;
            }
            else if (cmd_info.parse_function.index() == 1)
            {
                /** Calls command functions that receive a range iterator **/
                it = xtl::get<1>(cmd_info.parse_function)(static_cast<T*>(this),
                                                          it,
                                                          end);
            }

            return it;
        }

        /**
            Parse all tokens in the range of iterators passed
            Returns an iterator past the last successfully parsed token
        **/
        input_it parse_loop(const input_it& begin, const input_it& end)
        {
            /** First handle initial tokens **/
            input_it it = static_cast<T*>(this)->parse_init(begin, end);

            /** Then move on to the mapping table **/
            while (it != end)
            {
                input_it next = parse_step(it, end);

                if (next == it)
                {
                    break;
                }

                it = next;
            }

            return it;
        }
    };
}
