/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "xeus-zmq/xshell_runner.hpp"
#include "xeus-zmq/xserver_zmq_split.hpp"

namespace xeus
{

    void xsubshell_runner::register_server(xserver_zmq_split& server)
    {
        p_server = &server;
    }

    void xsubshell_runner::run()
    {
        run_impl();
    }

    auto create_runner(const std::string& subshell_id) const -> runner_ptr 
    {
        return create_runner_impl(subshell_id);
    }

    xsubshell_runner::xsubshell_runner(const std::string& subshell_id)
        : m_subshell_id(subshell_id)
    {
    }

    fd_t xsubshell_runner::get_subshell_fd() const
    {
        p_server->get_subshell_fd(m_subshell_id);
    }

    xserver_zmq_impl& get_server()
    {
        return *p_server;
    }

    std::optional<channel> xsubshell_runner::poll_channels(long timeout)
    {
        p_server->poll_channels(m_subshell_id, timeout);
    }

    std::optional<xmessage> xsubshell_runner::read_shell(int flags)
    {
        return p_server->read_shell(m_subshell_id, flags);
    }

    std::optional<std::string> xsubshell_runner::read_controller(int flags)
    {
        return p_server->read_shell_controller(m_subshell_id, flags);
    }

    void xsubshell_runner::send_controller(std::string message)
    {
        p_server->send_shell_controller(m_subshell_id, std::move(message));
    }

    void xsubshell_runner::notify_shell_listener(xmessage message)
    {
        p_server->notify_shell_listener(std::move(message));
    }

    std::string xsubshell_runner::notify_internal_listener(std::string message)
    {
        return p_server->notify_internal_listener(std::move(message));
    }
}
