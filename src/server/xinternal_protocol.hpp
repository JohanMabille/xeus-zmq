/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XINTERNAL_PROTOCOL_HPP
#define XINTERNAL_PROTOCOL_HPP

#include <string_view>

namespace xeus
{
    inline constexpr std::string_view SHELL_ROUTING_KEY = "shell";
    inline constexpr std::string_view CONTROL_ROUTING_KEY = "control";
    inline constexpr std::string_view STDIN_ROUTING_KEY = "stdin";

    inline constexpr std::string_view STOP_CMD = "stop";
    inline constexpr std::string_view ADD_SUBSHELL_CMD = "add_subshell";
    inline constexpr std::string_view REMOVE_SUBSHELL_CMD = "remove_subshell";
}

#endif
