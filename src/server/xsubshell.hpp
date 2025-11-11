/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SUBSHELL_HPP
#define XEUS_SUBSHELL_HPP

#include <queue>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "xeus/xmessage.hpp"
#include "xeus/xserver.hpp"

#include "xeus-zmq/xmiddleware.hpp"

namespace xeus
{
    class xzmq_server_split_impl;

    class xsubshell
    {
    public:

        using listener = std::function<void(xmessage)>;

        xsubshell(zmq::context& context,
                  const std::string& id,
                  xserver_zmq_split_impl* server);

        fd_t get_subshell_fd() const;

        std::optional<channel> poll_channels(long timeout);
        std::optional<xmessage> read_shell();
        std::optional<std::string> read_controller();

        void send_shell(zmq::multipart_t& message);
        void send_controller(std::string message);
        std::optional<xmessage> send_stdin(zmq::multipart_t& message);

        void publish(zmq::multipart_t& message);
        void abort_queue(const listener& l, long polling_interval);

    private:

        zmq::socket_t m_shell_poller;
        zmq::socket_t m_publisher_pub;

        xserver_zmq_split_impl* p_server;

        using message_queue = std::queue<zmq::multipart_t>;
        message_queue m_shell_queue;
        message_queue m_control_queue;

        zmq::multipart_t pop_queue(message_queue& queue);
    };
}

#endif
