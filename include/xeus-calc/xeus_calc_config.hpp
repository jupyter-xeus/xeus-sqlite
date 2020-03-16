/***************************************************************************
* Copyright (c) 2019, Sylvain Corlay, Johan Mabille, Wolf Vollprecht       *
* Copyright (c) 2019, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XCALC_CONFIG_HPP
#define XCALC_CONFIG_HPP

// Project version
#define XCALC_VERSION_MAJOR 0
#define XCALC_VERSION_MINOR 1
#define XCALC_VERSION_PATCH 0

// Composing the version string from major, minor and patch
#define XCALC_CONCATENATE(A, B) XCALC_CONCATENATE_IMPL(A, B)
#define XCALC_CONCATENATE_IMPL(A, B) A##B
#define XCALC_STRINGIFY(a) XCALC_STRINGIFY_IMPL(a)
#define XCALC_STRINGIFY_IMPL(a) #a

#define XCALC_VERSION XCALC_STRINGIFY(XCALC_CONCATENATE(XCALC_VERSION_MAJOR,   \
                 XCALC_CONCATENATE(.,XCALC_CONCATENATE(XCALC_VERSION_MINOR,   \
                                  XCALC_CONCATENATE(.,XCALC_VERSION_PATCH)))))

#ifdef _WIN32
    #ifdef XEUS_CALC_EXPORTS
        #define XEUS_CALC_API __declspec(dllexport)
    #else
        #define XEUS_CALC_API __declspec(dllimport)
    #endif
#else
    #define XEUS_CALC_API
#endif

#endif
