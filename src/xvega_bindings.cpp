/***************************************************************************
* Copyright (c) 2020, Mariana Meireles                                     *
* Copyright (c) 2020, QuantStack                                           *
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

#include "xeus-sqlite/xvega_bindings.hpp"
#include "xeus-sqlite/utils.hpp"

namespace nl = nlohmann;

namespace xeus_sqlite
{

    xv_sqlite::xv_sqlite(xv::Chart& chart) : chart(chart)
    {
    }

    xv_sqlite::~xv_sqlite()
    {
    }

    const std::map<std::string, xv_sqlite::command_info> xv_sqlite::xvega_mapping_table = {
        {"WIDTH", {1, &xv_sqlite::parse_width}},
        {"HEIGHT", {1, &xv_sqlite::parse_height}},
        {"X_FIELD", {1, &xv_sqlite::parse_x_field}},
        {"Y_FIELD", {1, &xv_sqlite::parse_y_field}},
        {"MARK", {1, &xv_sqlite::parse_mark}},
    };

    const std::map<std::string, xv_sqlite::command_info> xv_sqlite::mark_mapping_table = {
        {"COLOR", {1, &xv_sqlite::parse_color}}
    };


    xv_sqlite::input_it xv_sqlite::parse_width(const xv_sqlite::input_it& input)
    {
        this->chart.width() = std::stoi(*input);
        return input + 1;
    }

    xv_sqlite::input_it xv_sqlite::parse_height(const xv_sqlite::input_it& input)
    {
        this->chart.height() = std::stoi(*input);
        return input + 1;
    }

    xv_sqlite::input_it xv_sqlite::parse_x_field(const xv_sqlite::input_it& input)
    {
        xv::X x_enc = xv::X()
                        .field(*(input))
                        .type("quantitative");
        this->chart.encoding().value().x = x_enc;
        return input + 1;
    }

    xv_sqlite::input_it xv_sqlite::parse_y_field(const xv_sqlite::input_it& input)
    {
        xv::Y y_enc = xv::Y()
                        .field(*(input))
                        .type("quantitative");
        this->chart.encoding().value().y = y_enc;
        return input + 1;
    }

    xv_sqlite::input_it xv_sqlite::parse_mark(const xv_sqlite::input_it& input, const xv_sqlite::input_it& end)
    {
        //TODO: should only accept one attribute for marks
        const std::map<std::string, xv_sqlite::command_info> mark_attr_mapping_table = {
            {"ARC",    {1, [this]{ this->chart.mark() = xv::mark_arc();    }}},
            {"AREA",   {1, [this]{ this->chart.mark() = xv::mark_area();   }}},
            {"BAR",    {1, [this]{ this->chart.mark() = xv::mark_bar();    }}},
            {"CIRCLE", {1, [this]{ this->chart.mark() = xv::mark_circle(); }}},
            {"LINE",   {1, [this]{ this->chart.mark() = xv::mark_line();   }}},
            {"POINT",  {1, [this]{ this->chart.mark() = xv::mark_point();  }}},
            {"RECT",   {1, [this]{ this->chart.mark() = xv::mark_rect();   }}},
            {"RULE",   {1, [this]{ this->chart.mark() = xv::mark_rule();   }}},
            {"SQUARE", {1, [this]{ this->chart.mark() = xv::mark_square(); }}},
            {"TICK",   {1, [this]{ this->chart.mark() = xv::mark_tick();   }}},
            {"TRAIL",  {1, [this]{ this->chart.mark() = xv::mark_trail();  }}},
        };

        xvega_execution_loop(input, end, mark_attr_mapping_table);
        xvega_execution_loop(input, end, mark_mapping_table);
        return input + 1;
    }

    xv_sqlite::input_it xv_sqlite::parse_color(const xv_sqlite::input_it& input)
    {
        struct visitor
        {
            std::string m_color;

            visitor(const std::string& color): m_color(color)
            {
            }

            void operator()(xv::mark_arc& m)      { m.color = m_color; }
            void operator()(xv::mark_area& m)     { m.color = m_color; }
            void operator()(xv::mark_bar& m)      { m.color = m_color; }
            void operator()(xv::mark_circle& m)   { m.color = m_color; }
            void operator()(xv::mark_line& m)     { m.color = m_color; }
            void operator()(xv::mark_point& m)    { m.color = m_color; }
            void operator()(xv::mark_rect& m)     { m.color = m_color; }
            void operator()(xv::mark_rule& m)     { m.color = m_color; }
            void operator()(xv::mark_square& m)   { m.color = m_color; }
            void operator()(xv::mark_tick& m)     { m.color = m_color; }
            void operator()(xv::mark_trail& m)    { m.color = m_color; }
            void operator()(xv::mark_geoshape& m) { m.color = m_color; }
            void operator()(xv::mark_image& m)    { m.color = m_color; }
            void operator()(xv::mark_text& m)     { m.color = m_color; }
        };

        //TODO: validate input: color must be lowercase
        visitor v(*input);
        xtl::visit(v, this->chart.mark());
        return input + 1;
    }

    void xv_sqlite::xvega_execution_loop(const xv_sqlite::input_it& begin, const xv_sqlite::input_it& end, const std::map<std::string, xv_sqlite::command_info> mapping_table)
    {
        xv_sqlite::input_it it = begin;

        while (it != end)
        {
            auto cmd_it = mapping_table.find(*it);

            if (cmd_it == mapping_table.end())
            {
                // throw std::runtime_error("This is not a valid command for SQLite XVega.");
                ++it;
                continue;
            }

            xv_sqlite::command_info cmd_info = cmd_it->second;

            /* Prevents code to end prematurely.*/
            if (std::distance(it, end) < cmd_info.number_required_arguments)
            {
                throw std::runtime_error(std::string("Arguments missing."));
            }

            /* Advances to first parameter */
            ++it;

            //TODO: use a visitor instead of index
            //The impl. is too complex, don't think it's worth it, but should we?
            // struct visitor
            // {
            //     xv_sqlite* _this;
            //     const xv_sqlite::input_it& it;
            //     const xv_sqlite::input_it& end;

            //     visitor(xv_sqlite* _this,
            //             const xv_sqlite::input_it& it,
            //             const xv_sqlite::input_it& end)
            //                 : _this(_this)
            //                 , it(it)
            //                 , end(end)
            //     {
            //     }

            //     xtl::variant<std::monostate, xv_sqlite::input_it> operator()(xv_sqlite::point_it_fun f) { return f(*_this, it); }
            //     xtl::variant<std::monostate, xv_sqlite::input_it> operator()(xv_sqlite::range_it_fun f) { return f(*_this, it, end); }
            //     xtl::variant<std::monostate, xv_sqlite::input_it> operator()(xv_sqlite::free_fun f) { f(); }
            // };

            // visitor v(this, it, end);

            // xtl::variant<std::monostate, xv_sqlite::input_it> ret = xtl::visit(v, cmd_info.parse_function);

            // if (ret.index() == 1)
            // {
            //     it = xtl::get<1>(ret);
            // }

            /* Calls parsing function for command */
            if (cmd_info.parse_function.index() == 0)
            {
                /* Calls command functions that receive a point iterator*/
                it = xtl::get<0>(cmd_info.parse_function)(*this, it);
            }
            else if (cmd_info.parse_function.index() == 1)
            {
                /* Calls command functions that receive a range iterator */
                it = xtl::get<1>(cmd_info.parse_function)(*this, it, end);
            }
            else if (cmd_info.parse_function.index() == 2)
            {
                /* Calls command functions that receive a range iterator*/
                xtl::get<2>(cmd_info.parse_function)();
            }
        }
    }

    nl::json xv_sqlite::process_xvega_input(std::vector<std::string>
                                               tokenized_input,
                                               xv::df_type xv_sqlite_df)
    {
        /* Initializes and populates xeus_sqlite object*/
        xv::Chart chart;
        chart.encoding() = xv::Encodings();
        xv_sqlite context(chart);

        /* Populates chart with data gathered on interpreter::process_SQLite_input */
        xv::data_frame data_frame;
        data_frame.values = xv_sqlite_df;
        chart.data() = data_frame;

        context.xvega_execution_loop(tokenized_input.begin(),
                                     tokenized_input.end(),
                                     xvega_mapping_table);

        nl::json bundle;
        bundle["application/vnd.vegalite.v3+json"] = chart;

        return bundle;
    }

    std::pair<std::vector<std::string>, std::vector<std::string>> 
        xv_sqlite::split_xv_sqlite_input(std::vector<std::string> complete_input)
    {
        //TODO: test edge cases
        auto found = std::find(complete_input.begin(),
                               complete_input.end(),
                               "<>");

        std::vector<std::string> xvega_input(complete_input.begin(), found);
        std::vector<std::string> sqlite_input(found + 1, complete_input.end());

        return std::make_pair(xvega_input, sqlite_input);
    }
}
