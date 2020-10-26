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

#include "xvega/xvega.hpp"
#include "xeus_sqlite_config.hpp"

namespace nl = nlohmann;

namespace xeus_sqlite
{
    class XEUS_SQLITE_API xvega_sqlite
    {
    public:

        xvega_sqlite() = default;
        virtual ~xvega_sqlite() = default;

        static nl::json process_xvega_input(std::vector<std::string>, xv::df_type);
        static std::pair<std::vector<std::string>, std::vector<std::string>>
               split_xvega_sqlite_input(std::vector<std::string>);

        xv::df_type xvega_sqlite_df;

    };
}

#endif