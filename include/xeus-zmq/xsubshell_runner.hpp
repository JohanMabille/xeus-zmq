/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SUBSHELL_RUNNER_HPP
#define XEUS_SUBSHELL_RUNNER_HPP

#include <optional>
#include <string>

#include "xeus/xmessage.hpp"
#include "xeus/xserver.hpp"

#include "xeus-zmq.hpp"

namespace xeus
{

    class XEUS_ZMQ_API xsubshell_runner
    {
    public:

        virtual ~xsubshell_runner() = default;

        xsubshell_runner(const xsubshell_runner&) = delete;
        xsubshell_runner& operator=(const xsubshell_runner&) = delete;
        xsubshell_runner(xsubshell_runner&&) = delete;
        xsubshell_runner& operator=(xsubshell_runner&&) = delete;

        void register_server(xserver_zmq_split& server);
        void run();

        using runner_ptr = std::unique_ptr<xsubshell_runner>;
        runner_ptr create_runner(const std::string& subshell_id) const;

    protected:

        xsubshell_runner(const std::string& subshell_id);

        fd_t get_subshell_fd() const;
        xserver_zmq_split& get_server();

        std::optional<channel> poll_channels(long timeout = -1);
        std::optional<xmessage> read_shell(int flags = 0);
        std::optional<std::string> read_controller(int flags = 0);

        void send_controller(std::string message);

        void notify_shell_listener(xmessage message);
        std::string notify_internal_listener(std::string message);

    private:

        virtual void run_impl() = 0;
        virtual runner_ptr create_runner_impl(const std::string& subshell_id) const = 0;

        std::string m_subshell_id;
        xserver_zmq_split* p_server = nullptr;
    };
}

#endif
