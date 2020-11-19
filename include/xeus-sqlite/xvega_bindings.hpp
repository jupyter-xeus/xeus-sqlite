/***************************************************************************
* Copyright (c) 2020, QuantStack and Xeus-SQLite contributors              *
*                                                                          *
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
#include <utility>
#include <variant>
#include <vector>

#include "xvega/xvega.hpp"
#include "xeus_sqlite_config.hpp"

namespace nl = nlohmann;

namespace xeus_sqlite
{
    class XEUS_SQLITE_API xv_sqlite
    {
    public:

        virtual ~xv_sqlite();

        //TODO: either these classes can be moved to the cpp
        //or the relevant hpp parts on cpp moved here. I'm not sure what's the
        //best appproach
        static nl::json process_xvega_input(std::vector<std::string>, xv::df_type);
        static std::pair<std::vector<std::string>, std::vector<std::string>>
               split_xv_sqlite_input(std::vector<std::string>);
    };
}

#endif
