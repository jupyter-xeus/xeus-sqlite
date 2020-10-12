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

        auto mark = std::find(tokenized_input.begin(),
                              tokenized_input.end(),
                              "MARK");
        if (mark != tokenized_input.end())
        {
            //TODO: go throught list of different kinds of marks and let the
            //user actually choose which kind they want
            auto mp = xv::mark_point();
            chart.mark() = mp;
        }
        auto enc = xv::Encodings().x(x_enc).y(y_enc);
        chart.encoding() = enc;

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
