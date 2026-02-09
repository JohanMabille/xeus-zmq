/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "xeus-zmq/xsubshell_default_runner.hpp"

namespace xeus
{

    xsubshell_default_runner::xsubshell_default_runner(const std::string& subshell_id)
        : xsubshsell_runner(subshell_id)
    {
    }

    void xsubshell_default_runner::run_impl()
    {
        while(true)
        {
            auto chan = poll_channels();
            if (auto msg = read_shell(chan))
            {
                notify_shell_listener(std::move(msg.value()));
            }
            else if (auto msg = read_controller(chan))
            {
                std::string val = std::move(msg.value());
                if (val == "stop")
                {
                    send_controller(std::move(val));
                    break;
                }
                else
                {
                    std::string rep = notify_internal_listener(std::move(val));
                    send_controller(std::move(rep));
                }
            }
        }
    }

    runner_ptr xsubshell_default_runner::create_runner_impl(const std::string& subshell_id) const
    {
        runner_ptr res(new xsubshell_default_runner(subshell_id));
        res->register_server(get_server());
        return res;
    }

    std::optional<xmessage> xsubshell_default_runner::read_shell(optional_channel chan)
    {
        if (chan.has_value() && chan.value() == channel::SHELL)
        {
            return read_shell();
        }
        return std::nullopt;
    }

    std::optional<std::string> xsubshell_default_runner::read_control(optional_channel chan)
    {
        if (chan.has_value() && chan.value() == channel::CONTROL)
        {
            return read_control();
        }
        return std::nullopt;
    }
}
