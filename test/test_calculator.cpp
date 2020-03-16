#ifndef TEST_CALCULATOR_HPP
#define TEST_CALCULATOR_HPP

#include "gtest/gtest.h"

#include "xeus-calc/xeus_calc_interpreter.hpp"


namespace xeus_calc
{

    TEST(xeus_calc_interpreter, formating_expression)
    {
        std::string code = "73* 252 +42- 6";
        std::string formated_expr;
        formated_expr = formating_expr(code);
        EXPECT_EQ(formated_expr, "73 * 252 + 42 - 6");

        code = "1*3^(4+2)-4*(2+3*6^1)";
        formated_expr = formating_expr(code);
        EXPECT_EQ(formated_expr, "1 * 3 ^  ( 4 + 2 )  - 4 *  ( 2 + 3 * 6 ^ 1 ) ");
    }

    TEST(xeus_calc_interpreter, parse_RPN)
    {
        std::string code = "2*(33+14)-27/(4-1)";
        std::string parsed_expr;
        parsed_expr = parse_rpn(formating_expr(code));
        EXPECT_EQ(parsed_expr, "2 33 14 + * 27 4 1 - / - ");


        code = "1*3^(4+2)-4*(2+3*6^1)";
        parsed_expr = parse_rpn(formating_expr(code));
        EXPECT_EQ(parsed_expr, "1 3 4 2 + ^ * 4 2 3 6 1 ^ * + * - ");
    }

    TEST(xeus_calc_interpreter, compute_RPN)
    {
        std::string code = "14 + 2* (3+ 2) /2^ 2-3 +16";
        double result = compute_rpn(parse_rpn(formating_expr(code)));
        EXPECT_EQ(result, 29.5);

        code = "1*3^(4+2)-4*(2+3*6^1)";
        result = compute_rpn(parse_rpn(formating_expr(code)));
        EXPECT_EQ(result, 649);
    }

    TEST(xeus_calc_interpreter, wrong_character)
    {
        EXPECT_THROW({
            std::string code = "2&3+5";
            std::string formated_expr = formating_expr(code);
        }, std::runtime_error );
    }

    TEST(xeus_calc_interpreter, missing_parenthesis)
    {
        EXPECT_THROW({
            std::string code = "2-(3+5";
            std::string parsed_expr = parse_rpn(formating_expr(code));
        }, std::runtime_error );

        EXPECT_THROW({
            std::string code = "2-3+5)";
            std::string parsed_expr = parse_rpn(formating_expr(code));
        }, std::runtime_error );
    }

    TEST(xeus_calc_interpreter, missing_operand)
    {
        EXPECT_THROW({
            std::string code = "2+";
            double result = compute_rpn(parse_rpn(formating_expr(code)));
        }, std::runtime_error );

        EXPECT_THROW({
            std::string code = "-+";
            double result = compute_rpn(parse_rpn(formating_expr(code)));
        }, std::runtime_error );
    }
}

#endif
