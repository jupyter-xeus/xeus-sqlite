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
        {"COLOR", {1, &xv_sqlite::parse_mark_color}}
    };

    const std::map<std::string, xv_sqlite::command_info> xv_sqlite::field_mapping_table = {
        {"TYPE", {1, &xv_sqlite::parse_field_type}}
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

    xv_sqlite::input_it xv_sqlite::parse_x_field(const xv_sqlite::input_it& input, const xv_sqlite::input_it& end)
    {
        xv::X x_enc = xv::X()
                        .field(*(input))
                        .type("quantitative");
        this->chart.encoding().value().x = x_enc;

        std::cout << "it used inside parse_x_field " << *input << std::endl;

        xv_sqlite::input_it it = input;
        it += 1;
        bool succeeded;

        do
        {
            std::cout << "trying " << *it << " in field_mapping_table\n";
            std::tie(it, succeeded) = xvega_execution_step(it, end, field_mapping_table);
            std::cout << "this should be false: " << succeeded << std::endl;
        } while (succeeded);

        std::cout << "end parse_x_field at " << *it << "\n";

        return it;
    }

    xv_sqlite::input_it xv_sqlite::parse_y_field(const xv_sqlite::input_it& input, const xv_sqlite::input_it& end)
    {
        xv::Y y_enc = xv::Y()
                        .field(*(input))
                        .type("quantitative");
        this->chart.encoding().value().y = y_enc;
        // xvega_execution_step(input, end, field_mapping_table);
        return input + 1;
    }

    xv_sqlite::input_it xv_sqlite::parse_field_type(const xv_sqlite::input_it& input,
                                const xv_sqlite::input_it& end)
    {
        const std::map<std::string, xv_sqlite::command_info> field_type_mapping_table = {
            {"QUANTITATIVE", {0, [this]{ this->chart.encoding().value().x().value().type().value() = "quantitative"; }}},
            {"ORDINAL",      {0, [this]{ this->chart.encoding().value().x().value().type().value() = "ordinal"; }}},
        };


        xv_sqlite::input_it it = input;

        std::cout << "✨ parse_field_type began with it = " << *it <<std::endl;
        // it += 1;
        std::cout << "✨ parse_field_type current it = " << *it <<std::endl;

        std::tie(it, std::ignore) = xvega_execution_step(it, end, field_type_mapping_table);
        return input + 1;
    }

    xv_sqlite::input_it xv_sqlite::parse_mark(const xv_sqlite::input_it& input,
                                              const xv_sqlite::input_it& end)
    {
        //TODO: should only accept one attribute for marks
        const std::map<std::string, xv_sqlite::command_info> mark_attr_mapping_table = {
            {"ARC",    {0, [this]{ this->chart.mark() = xv::mark_arc();    }}},
            {"AREA",   {0, [this]{ this->chart.mark() = xv::mark_area();   }}},
            {"BAR",    {0, [this]{ this->chart.mark() = xv::mark_bar();    }}},
            {"CIRCLE", {0, [this]{ this->chart.mark() = xv::mark_circle(); }}},
            {"LINE",   {0, [this]{ this->chart.mark() = xv::mark_line();   }}},
            {"POINT",  {0, [this]{ this->chart.mark() = xv::mark_point();  }}},
            {"RECT",   {0, [this]{ this->chart.mark() = xv::mark_rect();   }}},
            {"RULE",   {0, [this]{ this->chart.mark() = xv::mark_rule();   }}},
            {"SQUARE", {0, [this]{ this->chart.mark() = xv::mark_square(); }}},
            {"TICK",   {0, [this]{ this->chart.mark() = xv::mark_tick();   }}},
            {"TRAIL",  {0, [this]{ this->chart.mark() = xv::mark_trail();  }}},
        };

        xv_sqlite::input_it it = input;
        bool succeeded;

        std::tie(it, std::ignore) = xvega_execution_step(input, end, mark_attr_mapping_table);

        do
        {
            std::tie(it, succeeded) = xvega_execution_step(it, end, mark_mapping_table);
        } while (succeeded);

        return it;
    }

    xv_sqlite::input_it xv_sqlite::parse_mark_color(const xv_sqlite::input_it& input)
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

    std::pair<xv_sqlite::input_it, bool> xv_sqlite::xvega_execution_step(const xv_sqlite::input_it& begin,
                                         const xv_sqlite::input_it& end,
                                         const std::map<std::string,
                                         xv_sqlite::command_info> mapping_table)
    {
        std::cout << "first line of xvega_execution_step\n";
        xv_sqlite::input_it it = begin;

        auto cmd_it = mapping_table.find(*it);


        if (cmd_it == mapping_table.end())
        {
            std::cout << "what makes things false "<< std::endl;
            return std::make_pair(it, false);
        }

        std::cout << "🌈 Current command: " << cmd_it->first <<std::endl;
        xv_sqlite::command_info cmd_info = cmd_it->second;

        /* Prevents code to end prematurely.*/
        if (std::distance(it, end) < cmd_info.number_required_arguments)
        {
            throw std::runtime_error(std::string("Arguments missing."));
        }

        /* Advances to next parameter if command has parameters */
        if (cmd_info.number_required_arguments > 0)
        {
            ++it;
        }
        std::cout << "🌈🌈 Current command: " << *it <<std::endl;
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
            /* Treat case where functions return void */
            xtl::get<2>(cmd_info.parse_function)();
            ++it;
        }

        return std::make_pair(it, true);

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

        //     /* Types must be variant of all possible outcomes because the visit function can't choose right type byt itself */
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
        input_it it = tokenized_input.begin();

        bool succeeded;

        /* Main execution loop */
        std::cout << "🌸 Main loop" <<std::endl;
        while (it != tokenized_input.end())
        {
            std::tie(it, succeeded) = 
                context.xvega_execution_step(it,
                                             tokenized_input.end(),
                                             xvega_mapping_table);

            if (!succeeded)
            {
                throw std::runtime_error("This is not a valid command for SQLite XVega.");
            }
        }

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
