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

#include "xeus-sqlite/xvega_bindings.hpp"

namespace nl = nlohmann;

namespace xeus_sqlite
{
    void xvega_sqlite::test()
    {
        std::cout << "🌸🌸🌸\n";
    }

    nl::json xvega_sqlite::parse_xvega_input(std::vector<std::string>
                                             tokenized_input,
                                             xv::df_type& xvega_sqlite_df)
    {
        for (auto i : tokenized_input)
        {
            std::cout << "🔥🔥" << i << std::endl;
        }

        /* Creates Chart object */
        xv::Chart chart;

        /* Converts the xv::df_type into a data_frame type that is compatible
            with xv::Chart
        */
        xv::data_frame data_frame;
        data_frame.values = xvega_sqlite_df;

        /* Populates chart with data gathered on interpreter::run_SQLite_code */
        chart.data() = data_frame;

        /* Creates basic xvega template */
        auto json_template = xv::base_vegalite_json();
        populate_data(json_template, chart);


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

        /* Sets mark configurations */
        int len_marks = chart.marks().size();
        if(len_marks > 1)
        {
            json_template["layer"] = {{}};
        }


        auto marks = std::find(tokenized_input.begin(),
                                 tokenized_input.end(),
                                 "MARKS");
        if (marks != tokenized_input.end())
        {
            //TODO: go throught list of different kinds of marks and let the
            //user actually choose which kind they want
            xv::mark_point mp;
            chart.marks() = {mp};
        }
        auto enc = xv::Encodings().x(x_enc).y(y_enc);
        chart.encodings() = {enc};

        populate_marks(json_template, chart);
        populate_encodings(json_template, chart);
        populate_selections(json_template, chart);
        populate_transformations(json_template, chart);

        nl::json bundle;
        bundle["application/vnd.vegalite.v3+json"] = json_template;
        return bundle;

    }

    void xvega_sqlite::run_xvega_code(std::string xvega_input, xv::df_type xvega_sqlite_df)
    {

    }
}
