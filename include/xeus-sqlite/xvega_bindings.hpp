/***************************************************************************
* Copyright (c) 2020, Mariana Meireles                                     *
* Copyright (c) 2020, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SQLITE_XVEGA_BINDINGS_HPP
#define XEUS_SQLITE_XVEGA_BINDINGS_HPP

#include <iterator>
#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "xvega/xvega.hpp"
#include "xeus_sqlite_config.hpp"

namespace nl = nlohmann;

namespace xeus_sqlite
{
    class XEUS_SQLITE_API xvega_sqlite
    {
    public:

        virtual ~xvega_sqlite();

        static nl::json process_xvega_input(std::vector<std::string>, xv::df_type);
        static std::pair<std::vector<std::string>, std::vector<std::string>>
               split_xvega_sqlite_input(std::vector<std::string>);

    private:

        xvega_sqlite(xv::Chart&);

        using input_it = std::vector<std::string>::iterator;
        using free_fun = std::function<void()>;
        using point_it_fun = std::function<input_it(xvega_sqlite&, const input_it&)>;
        using range_it_fun = std::function<input_it(xvega_sqlite&, const input_it&, const input_it&)>;
        using parse_function_types = xtl::variant<point_it_fun, range_it_fun, free_fun>;

        struct command_info {
            int number_required_arguments;
            parse_function_types parse_function;
        };

        static const std::map<std::string, command_info> xvega_mapping_table;
        static const std::map<std::string, command_info> mark_mapping_table;

        xv::Chart& chart;

        void xvega_execution_loop(const input_it&, const input_it&, const std::map<std::string, command_info>);

        input_it parse_width(const input_it&);
        input_it parse_height(const input_it&);
        input_it parse_x_field(const input_it&);
        input_it parse_y_field(const input_it&);

        input_it parse_mark(const input_it&, const input_it&);
        input_it parse_color(const input_it&);
    };
}

#endif