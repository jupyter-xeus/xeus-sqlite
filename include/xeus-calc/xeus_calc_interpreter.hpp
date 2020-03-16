/***************************************************************************
* Copyright (c) 2019, Sylvain Corlay, Johan Mabille, Wolf Vollprecht       *
* Copyright (c) 2019, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_CALC_INTERPRETER_HPP
#define XEUS_CALC_INTERPRETER_HPP

#include "xeus/xinterpreter.hpp"

#include "nlohmann/json.hpp"

#include "xeus_calc_config.hpp"

namespace nl = nlohmann;

namespace xeus_calc
{
    class XEUS_CALC_API interpreter : public xeus::xinterpreter
    {
    public:

        interpreter() = default;

        virtual ~interpreter() = default;

    private:

        void configure_impl() override;

        nl::json execute_request_impl(int execution_counter,
                                      const std::string& code,
                                      bool silent,
                                      bool store_history,
                                      nl::json user_expressions,
                                      bool allow_stdin) override;

        nl::json complete_request_impl(const std::string& code,
                                       int cursor_pos) override;

        nl::json inspect_request_impl(const std::string& code,
                                      int cursor_pos,
                                      int detail_level) override;

        nl::json is_complete_request_impl(const std::string& code) override;

        nl::json kernel_info_request_impl() override;

        void shutdown_request_impl() override;
    };

    using publish_type = std::function<void(const std::string& name, const std::string& text)>;

    XEUS_CALC_API std::string formating_expr(const std::string& expr);

    XEUS_CALC_API std::string parse_rpn(const std::string& infix, publish_type publish = [](const std::string& /*name*/, const std::string& /*text*/){});

    XEUS_CALC_API double compute_rpn(const std::string &expr, publish_type publish = [](const std::string& /*name*/, const std::string& /*text*/){});
}

#endif
