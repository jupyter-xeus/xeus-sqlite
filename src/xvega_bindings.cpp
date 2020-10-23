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

namespace nl = nlohmann;

namespace xeus_sqlite
{

    nl::json xvega_sqlite::run_xvega_input(std::vector<std::string>
                                             tokenized_input,
                                             xv::df_type xvega_sqlite_df)
    {
        xv::data_frame data_frame;
        data_frame.values = xvega_sqlite_df;

        /* Creates Chart object */
        xv::Chart chart;

        /* Creates basic xvega template */
        auto json_template = xv::base_vegalite_json();

        /* Populates chart with data gathered on interpreter::run_SQLite_code */
        chart.data() = data_frame;
        xv::populate_data(json_template, chart);

        /* Parses input and look for WIDTH and HEIGHT attrs */
        auto width = std::find(tokenized_input.begin(),
                               tokenized_input.end(),
                               "WIDTH");
        if (width != tokenized_input.end())
        {
            chart.width() = std::stoi(*(width + 1));
        }

        auto height = std::find(tokenized_input.begin(),
                                tokenized_input.end(),
                                "HEIGHT");
        if (height != tokenized_input.end())
        {
            chart.height() = std::stoi(*(height + 1));
            std::cout << chart.height() << std::endl;
        }

        /* Sets width and height attrs for the chart */
        xv::serialize(json_template, chart.width(), "width");
        xv::serialize(json_template, chart.height(), "height");

        /* Parses input and look for X_FIELD and Y_FIELD attrs */
        auto x_field = std::find(tokenized_input.begin(),
                                 tokenized_input.end(),
                                 "X_FIELD");
        xv::X x_enc;
        if (x_field != tokenized_input.end())
        {
            x_enc = xv::X().field(*(x_field + 1)).type("quantitative");
        }

        auto y_field = std::find(tokenized_input.begin(),
                                 tokenized_input.end(),
                                 "Y_FIELD");
        xv::Y y_enc;
        if (x_field != tokenized_input.end())
        {
            y_enc = xv::Y().field(*(y_field + 1)).type("quantitative");
        }
        /* Sets X_FIELD and Y_FIELD on chart */
        auto enc = xv::Encodings().x(x_enc).y(y_enc);
        chart.encoding() = enc;

        /* Parses input and look for MARK attr */
        auto mark = std::find(tokenized_input.begin(),
                              tokenized_input.end(),
                              "MARK");
        if (mark != tokenized_input.end())
        {
            //TODO: implement img and txt
            if (*(mark + 1) == "point")
            {
                auto m_point = xv::mark_point();
                chart.mark() = m_point;
            }

            else if (*(mark + 1) == "arc")
            {
                auto m_arc = xv::mark_arc();
                chart.mark() = m_arc;
            }

            else if (*(mark + 1) == "area")
            {
                auto m_area = xv::mark_area();
                chart.mark() = m_area;
            }

            else if (*(mark + 1) == "bar")
            {
                auto m_bar = xv::mark_bar();
                chart.mark() = m_bar;
            }

            else if (*(mark + 1) == "circle")
            {
                auto m_circle = xv::mark_circle();
                chart.mark() = m_circle;
            }

            // else if (*(mark + 1) == "geoshape")
            // {
            //     auto m_geoshape = xv::mark_geoshape();
            //     chart.mark() = m_geoshape;
            // }

            // else if (*(mark + 1) == "image")
            // {
            //     auto m_image = xv::mark_image();
            //     chart.mark() = m_image;
            // }

            else if (*(mark + 1) == "line")
            {
                auto m_line = xv::mark_line();
                chart.mark() = m_line;
            }

            else if (*(mark + 1) == "point")
            {
                auto m_point = xv::mark_point();
                chart.mark() = m_point;
            }

            else if (*(mark + 1) == "rect")
            {
                auto m_rect = xv::mark_rect();
                chart.mark() = m_rect;
            }

            else if (*(mark + 1) == "rule")
            {
                auto m_rule = xv::mark_rule();
                chart.mark() = m_rule;
            }

            else if (*(mark + 1) == "square")
            {
                auto m_square = xv::mark_square();
                chart.mark() = m_square;
            }

            // else if (*(mark + 1) == "text")
            // {
            //     auto m_text = xv::mark_text();
            //     chart.mark() = m_text;
            // }

            else if (*(mark + 1) == "tick")
            {
                auto m_tick = xv::mark_tick();
                chart.mark() = m_tick;
            }

            else if (*(mark + 1) == "trail")
            {
                auto m_trail = xv::mark_trail();
                chart.mark() = m_trail;
            }
        }

        populate_marks(json_template, chart);
        populate_encodings(json_template, chart);
        populate_selections(json_template, chart);
        populate_transformations(json_template, chart);

        nl::json bundle;
        bundle["application/vnd.vegalite.v3+json"] = json_template;

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
