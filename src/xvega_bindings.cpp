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

    //TODO: I don't think most final value() are necessary

    template<typename T>
    struct parser_base
    {
        using input_it = std::vector<std::string>::iterator;
        using point_it_fun = std::function<void(T*, const input_it&)>;
        using range_it_fun = std::function<input_it(T*,
                                                    const input_it&,
                                                    const input_it&)>;
        using parse_function_types = xtl::variant<point_it_fun,
                                                  range_it_fun>;

        struct command_info {
            int number_required_arguments;
            parse_function_types parse_function;
        };

        using free_fun = std::function<void()>;

        std::map<std::string, command_info> mapping_table;

        /* Switch implementation with strings */
        bool simple_switch(const std::string& token,
                           std::map<std::string,
                           free_fun> handlers)
        {
            auto handler_it = handlers.find(to_upper(token));
            if (handler_it == handlers.end()) {
                return false;
            }
            /* Call the handler */
            handler_it->second();
            return true;
        }

        /* Handle initial tokens before parse_loop takes over using the mapping
           table
        */
        input_it parse_init(const input_it& begin, const input_it& end)
        {
            return begin;
        }

        /* Parse a single token (and its dependencies)
           Returns an iterator past the last successfully parsed token
        */
        input_it parse_step(const input_it& begin, const input_it& end)
        {
            input_it it = begin;

            auto cmd_it = mapping_table.find(to_upper(*it));
            if (cmd_it == mapping_table.end())
            {
                return it; 
            }

            command_info cmd_info = cmd_it->second;

            /* Prevents code to end prematurely */
            if (std::distance(it, end) < cmd_info.number_required_arguments)
            {
                throw std::runtime_error("Arguments missing.");
            }

            /* Advances to next token */
            ++it;

            /* Calls parsing function for command */
            if (cmd_info.parse_function.index() == 0)
            {
                /* Calls command functions that receive a point iterator */
                xtl::get<0>(cmd_info.parse_function)(static_cast<T*>(this), it);
                it++;
            }
            else if (cmd_info.parse_function.index() == 1)
            {
                /* Calls command functions that receive a range iterator */
                it = xtl::get<1>(cmd_info.parse_function)(static_cast<T*>(this),
                                                          it,
                                                          end);
            }

            return it;
        }

        /* Parse all tokens in the range of iterators passed
           Returns an iterator past the last successfully parsed token
        */
        input_it parse_loop(const input_it& begin, const input_it& end)
        {
            /* First handle initial tokens */
            input_it it = static_cast<T*>(this)->parse_init(begin, end);

            /* Then move on to the mapping table */
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

    struct bin_parser : parser_base<bin_parser>
    {
        xv::Bin& bin;
        int num_parsed_attrs = 0;

        bin_parser(xv::Bin& bin) : bin(bin)
        {
            mapping_table = {
                {"ANCHOR",  { 1, &bin_parser::parse_bin_anchor  }},
                // {"BASE",    { 1, &bin_parser::parse_bin_base    }},
                // {"BINNED",  { 1, &bin_parser::parse_bin_binned  }},
                // {"DIVIDE",  { 1, &bin_parser::parse_bin_divide  }},
                // {"EXTENT",  { 1, &bin_parser::parse_bin_extent  }},
                // {"MAXBINS", { 1, &bin_parser::parse_bin_maxbins }},
                // {"MINSTEP", { 1, &bin_parser::parse_bin_minstep }},
                // {"NICE",    { 1, &bin_parser::parse_bin_nice    }},
                // {"STEP",    { 1, &bin_parser::parse_bin_step    }},
                // {"STEPS",   { 1, &bin_parser::parse_bin_steps   }},
            };
        }

        void parse_bin_anchor(const input_it& it)
        {
            bin.anchor().value() = std::stod(*it);
        }

    };

    struct field_parser : parser_base<field_parser>
    {
        using xy_variant = xtl::variant<xv::X*, xv::Y*>;
        xy_variant enc;

        field_parser(xy_variant enc) : enc(enc)
        {
            mapping_table = {
                {"TYPE",      { 1, &field_parser::parse_field_type      }},
                {"BIN",       { 1, &field_parser::parse_field_bin       }},
                {"AGGREGATE", { 1, &field_parser::parse_field_aggregate }},
                {"TIME_UNIT", { 1, &field_parser::parse_field_time_unit }},
            };
        }

        input_it parse_init(const input_it& begin, const input_it&)
        {
            xtl::visit([&](auto &&x_or_y)
            {
                x_or_y->field = *begin;
                x_or_y->type = "quantitative";
            }, enc);

            return begin + 1;
        }

        void parse_field_bin(const input_it& it)
        {
            bool found = xtl::visit([&](auto &&x_or_y)
            {
                return simple_switch(*it,
                {
                    {"TRUE",  [&]{ x_or_y->bin().value() = true; }},
                    {"FALSE", [&]{ x_or_y->bin().value() = false; }},
                });
            }, enc);
            if (!found)
            {
                xv::Bin bin;
                bin_parser parser(bin);
                parser.parse_loop();

                if (num_parsed_attrs == 0)
                {
                    throw std::runtime_error("Missing or invalid BIN type");
                }
            }
        }

        void parse_field_type(const input_it& it)
        {
            bool found = xtl::visit([&](auto &&x_or_y)
            {
                return simple_switch(*it,
                {
                    {"QUANTITATIVE", [&]{ x_or_y->type().value() = "quantitative"; }},
                    {"NOMINAL",      [&]{ x_or_y->type().value() = "nominal";      }},
                    {"ORDINAL",      [&]{ x_or_y->type().value() = "ordinal";      }},
                    {"TEMPORAL",     [&]{ x_or_y->type().value() = "temporal";     }},
                });
            }, enc);
            if (!found)
            {
                throw std::runtime_error("Missing or invalid TYPE type");
            }
        }

        input_it parse_field_aggregate(const input_it& begin, const input_it&)
        {
            bool found = xtl::visit([&](auto &&x_or_y)
            {
                return simple_switch(*begin,
                {
                    {"COUNT",     [&]{ x_or_y->aggregate().value() = "count";     }},
                    {"VALID",     [&]{ x_or_y->aggregate().value() = "valid";     }},
                    {"MISSING",   [&]{ x_or_y->aggregate().value() = "missing";   }},
                    {"DISTINCT",  [&]{ x_or_y->aggregate().value() = "distinct";  }},
                    {"SUM",       [&]{ x_or_y->aggregate().value() = "sum";       }},
                    {"PRODUCT",   [&]{ x_or_y->aggregate().value() = "product";   }},
                    {"MEAN",      [&]{ x_or_y->aggregate().value() = "mean";      }},
                    {"AVERAGE",   [&]{ x_or_y->aggregate().value() = "average";   }},
                    {"VARIANCE",  [&]{ x_or_y->aggregate().value() = "variance";  }},
                    {"VARIANCEP", [&]{ x_or_y->aggregate().value() = "variancep"; }},
                    {"STDEV",     [&]{ x_or_y->aggregate().value() = "stdev";     }},
                    {"STEDEVP",   [&]{ x_or_y->aggregate().value() = "stedevp";   }},
                    {"STEDERR",   [&]{ x_or_y->aggregate().value() = "stederr";   }},
                    {"MEDIAN",    [&]{ x_or_y->aggregate().value() = "median";    }},
                    {"Q1",        [&]{ x_or_y->aggregate().value() = "q1";        }},
                    {"Q3",        [&]{ x_or_y->aggregate().value() = "q3";        }},
                    {"CI0",       [&]{ x_or_y->aggregate().value() = "ci0";       }},
                    {"CI1",       [&]{ x_or_y->aggregate().value() = "ci1";       }},
                    {"MIN",       [&]{ x_or_y->aggregate().value() = "min";       }},
                    {"MAX",       [&]{ x_or_y->aggregate().value() = "max";       }},
                    {"ARGMIN",    [&]{ x_or_y->aggregate().value() = "argmin";    }},
                    {"ARGMAX",    [&]{ x_or_y->aggregate().value() = "argmax";    }},
                    //TODO: missing values arg
                });
            }, enc);
            if (!found)
            {
                throw std::runtime_error("Missing or invalid AGGREGATE type");
            }
            return begin + 1;
        }

        //TODO: must set title again, it disappears once you set the time unit
        // this problem might be related with the setting TITLE of the graph
        void parse_field_time_unit(const input_it& it)
        {
            bool found = xtl::visit([&](auto &&x_or_y)
            {
                return simple_switch(*it,
                {
                    {"YEAR",        [&]{ x_or_y->timeUnit().value() = "year";        }},
                    {"QUARTER",     [&]{ x_or_y->timeUnit().value() = "quarter";     }},
                    {"MONTH",       [&]{ x_or_y->timeUnit().value() = "month";       }},
                    {"DAY",         [&]{ x_or_y->timeUnit().value() = "day";         }},
                    {"DATE",        [&]{ x_or_y->timeUnit().value() = "date";        }},
                    {"HOURS",       [&]{ x_or_y->timeUnit().value() = "hours";       }},
                    {"MINUTES",     [&]{ x_or_y->timeUnit().value() = "minutes";     }},
                    {"SECONDS",     [&]{ x_or_y->timeUnit().value() = "seconds";     }},
                    {"MILISECONDS", [&]{ x_or_y->timeUnit().value() = "miliseconds"; }},
                });
            }, enc);
            if (!found)
            {
                throw std::runtime_error("Missing or invalid TIME_UNIT type");
            }
        }
    };

    struct mark_parser : parser_base<mark_parser>
    {
        xv::Chart& chart;

        mark_parser(xv::Chart& chart) : chart(chart)
        {
            mapping_table =
            {
                {"COLOR", { 1, &mark_parser::parse_color }},
            };
        }

        input_it parse_init(const input_it& begin, const input_it&)
        {
            bool found = simple_switch(*begin,
            {
                {"ARC",    [&]{ this->chart.mark() = xv::mark_arc();    }},
                {"AREA",   [&]{ this->chart.mark() = xv::mark_area();   }},
                {"BAR",    [&]{ this->chart.mark() = xv::mark_bar();    }},
                {"CIRCLE", [&]{ this->chart.mark() = xv::mark_circle(); }},
                {"LINE",   [&]{ this->chart.mark() = xv::mark_line();   }},
                {"POINT",  [&]{ this->chart.mark() = xv::mark_point();  }},
                {"RECT",   [&]{ this->chart.mark() = xv::mark_rect();   }},
                {"RULE",   [&]{ this->chart.mark() = xv::mark_rule();   }},
                {"SQUARE", [&]{ this->chart.mark() = xv::mark_square(); }},
                {"TICK",   [&]{ this->chart.mark() = xv::mark_tick();   }},
                {"TRAIL",  [&]{ this->chart.mark() = xv::mark_trail();  }},
            });
            if (!found)
            {
                throw std::runtime_error("Missing or invalid MARK type");
            }
            return begin + 1;
        }

        void parse_color(const input_it& it)
        {
            xtl::visit([&](auto&& mark_generic)
            {
                mark_generic.color = to_lower(*it);
            }, this->chart.mark());
        }
    };

    struct xv_sqlite_parser : parser_base<xv_sqlite_parser>
    {
        xv::Chart& chart;

        xv_sqlite_parser(xv::Chart& chart) : chart(chart)
        {
            mapping_table = {
                {"WIDTH",   { 1, &xv_sqlite_parser::parse_width   }},
                {"HEIGHT",  { 1, &xv_sqlite_parser::parse_height  }},
                {"X_FIELD", { 1, &xv_sqlite_parser::parse_x_field }},
                {"Y_FIELD", { 1, &xv_sqlite_parser::parse_y_field }},
                {"MARK",    { 1, &xv_sqlite_parser::parse_mark    }},
                {"GRID",    { 1, &xv_sqlite_parser::parse_grid    }},
                {"TITLE",   { 1, &xv_sqlite_parser::parse_title   }},
            };
        }

        input_it parse_init(const input_it& begin, const input_it&)
        {
            auto ac = xv::axis_config().grid(true);
            auto cf = xv::Config().axis(ac);
            this->chart.config() = cf;
            return begin;
        }

        void parse_width(const input_it& it)
        {
            this->chart.width() = std::stoi(*it);
        }

        void parse_height(const input_it& it)
        {
            this->chart.height() = std::stoi(*it);
        }

        input_it parse_x_field(const input_it& begin, const input_it& end)
        {
            std::vector<std::string> v;
            auto ax = xv::Axis().title(v);
            xv::X x_enc = xv::X().axis(ax);
            this->chart.encoding().value().x = x_enc;

            field_parser parser(&chart.encoding().value().x().value());
            return parser.parse_loop(begin, end);
        }

        input_it parse_y_field(const input_it& begin, const input_it& end)
        {
            xv::Y y_enc = xv::Y();
            this->chart.encoding().value().y = y_enc;

            field_parser parser(&chart.encoding().value().y().value());
            return parser.parse_loop(begin, end);
        }

        input_it parse_mark(const input_it& input, const input_it& end)
        {
            mark_parser parser(chart);
            return parser.parse_loop(input, end);
        }

        void parse_grid(const input_it& it)
        {

            bool found = simple_switch(*it,
                {
                    {"TRUE",  [&]{ this->chart.config().value().axis().value().grid() = true;  }},
                    {"FALSE", [&]{ this->chart.config().value().axis().value().grid() = false; }},
                });
            if (!found)
            {
                throw std::runtime_error("Missing or invalid GRID type");
            }
        }

        //TODO: not working
        void parse_title(const input_it& it)
        {
            std::vector<std::string> v = {*it};
            this->chart.config().value().axis().value().title() = v;
        }
    };

    nl::json xv_sqlite::process_xvega_input(std::vector<std::string>
                                               tokenized_input,
                                               xv::df_type xv_sqlite_df)
    {
        /* Initializes and populates xeus_sqlite object */
        xv::Chart chart;
        chart.encoding() = xv::Encodings();

        /* Populates chart with data gathered on interpreter::process_SQLite_input */
        xv::data_frame data_frame;
        data_frame.values = xv_sqlite_df;
        chart.data() = data_frame;

        /* Parse XVEGA_PLOT syntax */
        xv_sqlite_parser parser(chart);
        auto last_parsed = parser.parse_loop(tokenized_input.begin(),
                                             tokenized_input.end());
        if (last_parsed != tokenized_input.end())
        {
            throw std::runtime_error("This is not a valid command for SQLite XVega.");
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
