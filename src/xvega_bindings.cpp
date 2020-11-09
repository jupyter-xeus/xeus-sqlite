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
        {"WIDTH",   { 1, &xv_sqlite::parse_width   }},
        {"HEIGHT",  { 1, &xv_sqlite::parse_height  }},
        {"X_FIELD", { 1, &xv_sqlite::parse_x_field }},
        {"Y_FIELD", { 1, &xv_sqlite::parse_y_field }},
        {"MARK",    { 1, &xv_sqlite::parse_mark    }},
    };

    const std::map<std::string, xv_sqlite::command_info> xv_sqlite::mark_mapping_table = {
        {"COLOR", { 1, &xv_sqlite::parse_mark_color }}
    };

    const std::map<std::string, xv_sqlite::command_info> xv_sqlite::x_field_mapping_table = {
        {"TYPE",      { 1, &xv_sqlite::parse_x_field_type      }},
        {"BIN",       { 1, &xv_sqlite::parse_x_field_bin       }},
        {"AGGREGATE", { 1, &xv_sqlite::parse_x_field_aggregate }},
        // {"TITLE",     { 1, &xv_sqlite::parse_x_field_title     }},
        // {"GRID",      { 1, &xv_sqlite::parse_x_field_grid      }},
    };

    const std::map<std::string, xv_sqlite::command_info> xv_sqlite::y_field_mapping_table = {
        {"TYPE", { 1, &xv_sqlite::parse_y_field_type }},
        {"BIN",  { 1, &xv_sqlite::parse_y_field_bin  }},
        {"AGGREGATE", { 1, &xv_sqlite::parse_y_field_aggregate }},
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

    xv_sqlite::input_it xv_sqlite::parse_x_field(const xv_sqlite::input_it& input,
                                                 const xv_sqlite::input_it& end)
    {
        xv::X x_enc = xv::X()
                        .field(*(input))
                        .type("quantitative");
        this->chart.encoding().value().x = x_enc;

        xv_sqlite::input_it it = input + 1;
        bool succeeded;

        do
        {
            std::tie(it, succeeded) = xvega_execution_step(it, end, x_field_mapping_table);
        } while (succeeded);

        return it;
    }

    xv_sqlite::input_it xv_sqlite::parse_x_field_type(const xv_sqlite::input_it& input,
                                                      const xv_sqlite::input_it& end)
    {
        const std::map<std::string, xv_sqlite::command_info> x_field_type_mapping_table = {
            {"QUANTITATIVE", { 0, [this]{ this->chart.encoding().value().x().value().type().value() = "quantitative"; }}},
            {"ORDINAL",      { 0, [this]{ this->chart.encoding().value().x().value().type().value() = "ordinal";      }}},
            {"NOMINAL",      { 0, [this]{ this->chart.encoding().value().x().value().type().value() = "nominal";      }}},
        };

        xv_sqlite::input_it it = input;

        std::tie(it, std::ignore) = xvega_execution_step(it, end, x_field_type_mapping_table);

        return it;
    }

    xv_sqlite::input_it xv_sqlite::parse_x_field_bin(const xv_sqlite::input_it& input,
                                                     const xv_sqlite::input_it& end)
    {
        const std::map<std::string, xv_sqlite::command_info> x_field_bin_mapping_table = {
            {"TRUE",  { 0, [this]{ this->chart.encoding().value().x().value().bin().value() = true;  }}},
            {"FALSE", { 0, [this]{ this->chart.encoding().value().x().value().bin().value() = false; }}},
        };

        xv_sqlite::input_it it = input + 1;
        std::tie(it, std::ignore) = xvega_execution_step(it, end, x_field_bin_mapping_table);

        return it;
    }

    xv_sqlite::input_it xv_sqlite::parse_x_field_aggregate(const xv_sqlite::input_it& input,
                                                           const xv_sqlite::input_it& end)
    {
        const std::map<std::string, xv_sqlite::command_info> x_field_aggregate_mapping_table = {
            {"COUNT",     { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "count";     }}},
            {"VALID",     { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "valid";     }}},
            {"MISSING",   { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "missing";   }}},
            {"DISTINCT",  { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "distinct";  }}},
            {"SUM",       { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "sum";       }}},
            {"PRODUCT",   { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "product";   }}},
            {"MEAN",      { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "mean";      }}},
            {"AVERAGE",   { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "average";   }}},
            {"VARIANCE",  { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "variance";  }}},
            {"VARIANCEP", { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "variancep"; }}},
            {"STDEV",     { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "stdev";     }}},
            {"STEDEVP",   { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "stedevp";   }}},
            {"STEDERR",   { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "stederr";   }}},
            {"MEDIAN",    { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "median";    }}},
            {"Q1",        { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "q1";        }}},
            {"Q3",        { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "q3";        }}},
            {"CI0",       { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "ci0";       }}},
            {"CI1",       { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "ci1";       }}},
            {"MIN",       { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "min";       }}},
            {"MAX",       { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "max";       }}},
            {"ARGMIN",    { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "argmin";    }}},
            {"ARGMAX",    { 0, [this]{ this->chart.encoding().value().x().value().aggregate().value() = "argmax";    }}},
            //TODO: missing values arg
        };

        xv_sqlite::input_it it = input + 1;
        std::tie(it, std::ignore) = xvega_execution_step(it, end, x_field_aggregate_mapping_table);

        return it;
    }

    // //TODO: I don't know why it doesn't work
    // xv_sqlite::input_it xv_sqlite::parse_x_field_title(const xv_sqlite::input_it& input)
    // {
    //     xv_sqlite::input_it it = input;
    //     std::vector<std::string> v = {*it};
    //     auto x_axis = xtl::get<0>(this->chart.encoding().value().x().value().axis().value());
    //     x_axis.title().value() = v;
    //     this->chart.encoding().value().x().value().axis().value() = x_axis;
    //     return it + 1;
    // }

    // //TODO: I don't know if this is working
    // xv_sqlite::input_it xv_sqlite::parse_x_field_grid(const xv_sqlite::input_it& input,
    //                                                   const xv_sqlite::input_it& end)
    // {
    //     const std::map<std::string, xv_sqlite::command_info> x_field_grid_mapping_table = {
    //         // {"TRUE",  { 0, [this]{ xtl::get<0>(this->chart.encoding().value().x().value().axis().value()).grid().value() = true; std::cout << "🐉TRU🐉\n"; }}},
    //         {"FALSE", { 0, [this]{ 
    //             auto ac = xv::axis_config().grid(false);
    //             auto cf = xv::Config().axis(ac);
    //             this->chart.config() = cf;
    //             std::cout << "sets grid to false\n";}}},
    //     };

    //     xv_sqlite::input_it it = input;
    //     std::tie(it, std::ignore) = xvega_execution_step(it, end, x_field_grid_mapping_table);

    //     return it;
    // }

    xv_sqlite::input_it xv_sqlite::parse_y_field(const xv_sqlite::input_it& input,
                                                 const xv_sqlite::input_it& end)
    {
        xv::Y y_enc = xv::Y()
                        .field(*(input))
                        .type("quantitative");
        this->chart.encoding().value().y = y_enc;

        xv_sqlite::input_it it = input + 1;
        bool succeeded;

        do
        {
            std::tie(it, succeeded) = xvega_execution_step(it, end, y_field_mapping_table);
        } while (succeeded);

        return it;
    }

    xv_sqlite::input_it xv_sqlite::parse_y_field_type(const xv_sqlite::input_it& input,
                                                      const xv_sqlite::input_it& end)
    {
        const std::map<std::string, xv_sqlite::command_info> y_field_type_mapping_table = {
            {"QUANTITATIVE", { 0, [this]{ this->chart.encoding().value().y().value().type().value() = "quantitative"; }}},
            {"ORDINAL",      { 0, [this]{ this->chart.encoding().value().y().value().type().value() = "ordinal";      }}},
            {"NOMINAL",      { 0, [this]{ this->chart.encoding().value().y().value().type().value() = "nominal";      }}},
        };

        xv_sqlite::input_it it = input;

        std::tie(it, std::ignore) = xvega_execution_step(it, end, y_field_type_mapping_table);
        return it;
    }

    xv_sqlite::input_it xv_sqlite::parse_y_field_bin(const xv_sqlite::input_it& input,
                                                     const xv_sqlite::input_it& end)
    {
        const std::map<std::string, xv_sqlite::command_info> y_field_bin_mapping_table = {
            {"TRUE",  { 0, [this]{ this->chart.encoding().value().y().value().bin().value() = true;  }}},
            {"FALSE", { 0, [this]{ this->chart.encoding().value().y().value().bin().value() = false; }}}
        };

        xv_sqlite::input_it it = input + 1;
        std::tie(it, std::ignore) = xvega_execution_step(it, end, y_field_bin_mapping_table);

        return it;
    }

    xv_sqlite::input_it xv_sqlite::parse_y_field_aggregate(const xv_sqlite::input_it& input,
                                                           const xv_sqlite::input_it& end)
    {
        const std::map<std::string, xv_sqlite::command_info> y_field_aggregate_mapping_table = {
            {"COUNT",     { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "count";     }}},
            {"VALID",     { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "valid";     }}},
            {"MISSING",   { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "missing";   }}},
            {"DISTINCT",  { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "distinct";  }}},
            {"SUM",       { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "sum";       }}},
            {"PRODUCT",   { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "product";   }}},
            {"MEAN",      { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "mean";      }}},
            {"AVERAGE",   { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "average";   }}},
            {"VARIANCE",  { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "variance";  }}},
            {"VARIANCEP", { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "variancep"; }}},
            {"STDEV",     { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "stdev";     }}},
            {"STEDEVP",   { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "stedevp";   }}},
            {"STEDERR",   { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "stederr";   }}},
            {"MEDIAN",    { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "median";    }}},
            {"Q1",        { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "q1";        }}},
            {"Q3",        { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "q3";        }}},
            {"CI0",       { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "ci0";       }}},
            {"CI1",       { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "ci1";       }}},
            {"MIN",       { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "min";       }}},
            {"MAX",       { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "max";       }}},
            {"ARGMIN",    { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "argmin";    }}},
            {"ARGMAX",    { 0, [this]{ this->chart.encoding().value().y().value().aggregate().value() = "argmax";    }}},
            //TODO: missing values arg
        };

        xv_sqlite::input_it it = input + 1;
        std::tie(it, std::ignore) = xvega_execution_step(it, end, y_field_aggregate_mapping_table);

        return it;
    }


    xv_sqlite::input_it xv_sqlite::parse_mark(const xv_sqlite::input_it& input,
                                              const xv_sqlite::input_it& end)
    {
        //TODO: should only accept one attribute for marks
        const std::map<std::string, xv_sqlite::command_info> mark_attr_mapping_table = {
            {"ARC",    { 0, [this]{ this->chart.mark() = xv::mark_arc();    }}},
            {"AREA",   { 0, [this]{ this->chart.mark() = xv::mark_area();   }}},
            {"BAR",    { 0, [this]{ this->chart.mark() = xv::mark_bar();    }}},
            {"CIRCLE", { 0, [this]{ this->chart.mark() = xv::mark_circle(); }}},
            {"LINE",   { 0, [this]{ this->chart.mark() = xv::mark_line();   }}},
            {"POINT",  { 0, [this]{ this->chart.mark() = xv::mark_point();  }}},
            {"RECT",   { 0, [this]{ this->chart.mark() = xv::mark_rect();   }}},
            {"RULE",   { 0, [this]{ this->chart.mark() = xv::mark_rule();   }}},
            {"SQUARE", { 0, [this]{ this->chart.mark() = xv::mark_square(); }}},
            {"TICK",   { 0, [this]{ this->chart.mark() = xv::mark_tick();   }}},
            {"TRAIL",  { 0, [this]{ this->chart.mark() = xv::mark_trail();  }}},
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

    std::pair<xv_sqlite::input_it, bool> xv_sqlite::xvega_execution_step(
                                         const xv_sqlite::input_it& begin,
                                         const xv_sqlite::input_it& end,
                                         const std::map<std::string,
                                         xv_sqlite::command_info> mapping_table)
    {
        xv_sqlite::input_it it = begin;

        auto cmd_it = mapping_table.find(to_upper(*it));


        if (cmd_it == mapping_table.end())
        {
            return std::make_pair(it, false); 
        }

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
