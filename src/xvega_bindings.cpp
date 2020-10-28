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

    xvega_sqlite::xvega_sqlite(xv::Chart& chart) : chart(chart)
    {
    }

    xvega_sqlite::~xvega_sqlite()
    {
    }

    const std::map<std::string, xvega_sqlite::command_info> xvega_sqlite::mapping_table = {
        {"WIDTH", {1, &xvega_sqlite::parse_width}},
        {"HEIGHT", {1, &xvega_sqlite::parse_height}},
        {"X_FIELD", {1, &xvega_sqlite::parse_x_field}},
        {"Y_FIELD", {1, &xvega_sqlite::parse_y_field}},
    };

    xvega_sqlite::input_it xvega_sqlite::parse_width(const xvega_sqlite::input_it& input)
    {
        this->chart.width() = std::stoi(*input);
        return input + 1;
    }

    xvega_sqlite::input_it xvega_sqlite::parse_height(const xvega_sqlite::input_it& input)
    {
        this->chart.height() = std::stoi(*input);
        return input + 1;
    }

    xvega_sqlite::input_it xvega_sqlite::parse_x_field(const xvega_sqlite::input_it& input)
    {
        xv::X x_enc = xv::X()
            .field(*(input))
            .type("quantitative");
        this->chart.encoding().value().x = x_enc;
        return input + 1;
    }

    xvega_sqlite::input_it xvega_sqlite::parse_y_field(const xvega_sqlite::input_it& input)
    {
        xv::Y y_enc = xv::Y()
            .field(*(input))
            .type("quantitative");
        this->chart.encoding().value().y = y_enc;
        return input + 1;
    }

    nl::json xvega_sqlite::process_xvega_input(std::vector<std::string>
                                             tokenized_input,
                                             xv::df_type xvega_sqlite_df)
    {
        xv::data_frame data_frame;
        data_frame.values = xvega_sqlite_df;

        xv::Chart chart;
        chart.encoding() = xv::Encodings();

        xvega_sqlite context(chart);

        /* Populates chart with data gathered on interpreter::process_SQLite_input */
        chart.data() = data_frame;

        /* Remove XVEGA_PLOT command */
        tokenized_input.erase(tokenized_input.begin());

        xvega_sqlite::input_it it = tokenized_input.begin();
        while (it != tokenized_input.end()) {
            std::cout << *it << "🌈\n";
            auto cmdit = xvega_sqlite::mapping_table.find(*it);
            if (cmdit == xvega_sqlite::mapping_table.end()) {
                // throw std::runtime_error("This is not a valid command for SQLite XVega.");
                ++it;
                continue;
            }

            xvega_sqlite::command_info cmdinfo = cmdit->second;
            if (std::distance(it, tokenized_input.end()) <= cmdinfo.number_required_arguments) {
                throw std::runtime_error(std::string("This is not the right number of required arguments for the command ") + *it);
            }

            // Advance to first parameter
            ++it;

            // Call parsing function for command
            it = cmdinfo.parse_function(context, it);
        }

        /* Parses input and look for COLOR attr */
        auto color = std::find(tokenized_input.begin(),
                              tokenized_input.end(),
                              "COLOR");

        /* Parses input and look for MARK attr */
        auto mark = std::find(tokenized_input.begin(),
                              tokenized_input.end(),
                              "MARK");
        if (mark != tokenized_input.end())
        {
            //TODO: implement img, geoshape and txt
            if (case_insentive_equals(*(mark + 1), "POINT"))
            {
                auto m_point = xv::mark_point();
                if (color != tokenized_input.end())
                {
                    m_point.color = *(color + 1);
                }
                chart.mark() = m_point;
            }

            else if (case_insentive_equals(*(mark + 1), "ARC"))
            {
                auto m_arc = xv::mark_arc();
                if (color != tokenized_input.end())
                {
                    m_arc.color = *(color + 1);
                }
                chart.mark() = m_arc;
            }

            else if (case_insentive_equals(*(mark + 1), "AREA"))
            {
                auto m_area = xv::mark_area();
                if (color != tokenized_input.end())
                {
                    m_area.color = *(color + 1);
                }
                chart.mark() = m_area;
            }

            else if (case_insentive_equals(*(mark + 1), "BAR"))
            {
                auto m_bar = xv::mark_bar();
                if (color != tokenized_input.end())
                {
                    m_bar.color = *(color + 1);
                }
                chart.mark() = m_bar;
            }

            else if (case_insentive_equals(*(mark + 1), "CIRCLE"))
            {
                auto m_circle = xv::mark_circle();
                if (color != tokenized_input.end())
                {
                    m_circle.color = *(color + 1);
                }
                chart.mark() = m_circle;
            }

            // else if (case_insentive_equals(*(mark + 1), "geoshape"))
            // {
            //     auto m_geoshape = xv::mark_geoshape();
            // if (color != tokenized_input.end())
            //     {
            //         m_geoshape.color = *(color + 1);
            //     }
            //     chart.mark() = m_geoshape;
            // }

            // else if (case_insentive_equals(*(mark + 1), "image"))
            // {
            //     auto m_image = xv::mark_image();
            // if (color != tokenized_input.end())
            //     {
            //         m_image.color = *(color + 1);
            //     }
            //     chart.mark() = m_image;
            // }

            else if (case_insentive_equals(*(mark + 1), "LINE"))
            {
                auto m_line = xv::mark_line();
                if (color != tokenized_input.end())
                {
                    m_line.color = *(color + 1);
                }
                chart.mark() = m_line;
            }

            else if (case_insentive_equals(*(mark + 1), "POINT"))
            {
                auto m_point = xv::mark_point();
                if (color != tokenized_input.end())
                {
                    m_point.color = *(color + 1);
                }
                chart.mark() = m_point;
            }

            else if (case_insentive_equals(*(mark + 1), "RECT"))
            {
                auto m_rect = xv::mark_rect();
                if (color != tokenized_input.end())
                {
                    m_rect.color = *(color + 1);
                }
                chart.mark() = m_rect;
            }

            else if (case_insentive_equals(*(mark + 1), "RULE"))
            {
                auto m_rule = xv::mark_rule();
                if (color != tokenized_input.end())
                {
                    m_rule.color = *(color + 1);
                }
                chart.mark() = m_rule;
            }

            else if (case_insentive_equals(*(mark + 1), "square"))
            {
                auto m_square = xv::mark_square();
                if (color != tokenized_input.end())
                {
                    m_square.color = *(color + 1);
                }
                chart.mark() = m_square;
            }

            // else if (case_insentive_equals(*(mark + 1), "text"))
            // {
            //     auto m_text = xv::mark_text();
            // if (color != tokenized_input.end())
            //     {
            //         m_text.color = *(color + 1);
            //     }
            //     chart.mark() = m_text;
            // }

            else if (case_insentive_equals(*(mark + 1), "TICK"))
            {
                auto m_tick = xv::mark_tick();
                if (color != tokenized_input.end())
                {
                    m_tick.color = *(color + 1);
                }
                chart.mark() = m_tick;
            }

            else if (case_insentive_equals(*(mark + 1), "TRAIL"))
            {
                auto m_trail = xv::mark_trail();
                if (color != tokenized_input.end())
                {
                    m_trail.color = *(color + 1);
                }
                chart.mark() = m_trail;
            }
        }

        nl::json bundle;
        bundle["application/vnd.vegalite.v3+json"] = chart;

        return bundle;
    }

    std::pair<std::vector<std::string>, std::vector<std::string>> 
        xvega_sqlite::split_xvega_sqlite_input(std::vector<std::string> complete_input)
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
