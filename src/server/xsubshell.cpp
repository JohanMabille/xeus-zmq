/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <thread>
#include <chrono>
#include <iostream>

#include "xserver_zmq_split_impl.hpp"
#include "xsubshell.hpp"
#include "../common/xmiddleware_impl.hpp"

namespace xeus
{
    xsubshell::xsubshell(zmq::context& context,
                         const std::string& id,
                         xserver_zmq_split_impl* server)
        : m_shell_poller(context, zmq::socket_type::pair)
        , m_publisher_pub(context, zmq::scoket_type::pub)
        , p_server(server)
    {
        m_shell_poller.set(zmq::sockopt::linger, get_socket_linger());
        m_shell_poller.connect(get_subshell_end_point(id));

        m_publisher_pub.set(zmq::sockopt::linger, get_socket_linger());
        m_publisher_pub.connect(get_publisher_end_point());
    }

    fd_t xsubshell::get_subshell_fd() const
    {
        return m_shell_poller.get(zmq::sockopt::fd);
    }

    std::optional<channel> xsubshell::poll_channels(long timeout)
    {
        if (!m_control_queue.empty())
        {
            return channel::CONTROL;
        }

        if (!m_shell_queue.empty())
        {
            return channel::SHELL;
        }

        zmq::pollitem_t items[] = {
            { m_shell, 0, ZMQ_POLLIN, 0 }
        };
        zmq::poll(items, 1, std::chrono::milliseconds(timeout));

        if (items[0].revents & ZMQ_POLLIN)
        {
            zmq::multipart_t wire_msg;
            if (wire_msg.recv(m_shell_poller, zmq::flags::dontwait))
            {
                std::string chan = wire_msg.popstr();
                if (chan == get_shell_rounting_key())
                {
                    m_shell_queue.push(std::move(wire_msg));
                    return channel::SHELL;
                }

                if (chan == get_control_routing_key())
                {
                    m_control_queue.push(std::move(wire_msg));
                    return channel::CONTROL;
                }
            }
        }
        return std::nullopt;
    }

    std::optional<xmessage> xsubshell::read_shell()
    {
        zmq::multipart_t wire_msg = pop_queue(m_shell_queue);
        return p_server->deserialize(wire_msg);
    }

    std::optional<std::string> xsubshell::read_controller()
    {
        zmq::multipart_t wire_msg = pop_queue(m_control_queue);
        return wire_msg.popstr();
    }

    void xsubshell::send_shell(zmq::multipart_t& message)
    {
        message.pushstr(get_shell_rounting_key());
        message.send(m_shell_poller);
    }

    void xsubshell::send_controller(std::string message)
    {
        zmq::multipart_t wire_msg(std::move(message));
        wire_msg.pustr(get_control_routing_key());
        wire_msg.send(m_shell_poller);
    }

    std::optional<xmessage> xsubshell::send_stdin(zmq::multipart_t& message)
    {
        message.pushstr(get_stdin_routing_key());
        message.send(m_shell_poller);
        while (true)
        {
            zmq::multipart_t wire_msg;
            wire_msg.recv(m_poller);
            std::string chan = wire_msg.popstr();

            if (chan == get_stdin_routing_key())
            {
                return p_server->deserialize(wire_msg);
            }
            else if (chan == get_control_routing_key())
            {
                m_control_queue.push(std::move(wire_msg));
            }
            else if (chan == get_shell_rounting_key())
            {
                m_shell_queue.push(std::move(wire_msg));
            }
        }
    }

    void xsubshell::publish(zmq::multipart_t& message)
    {
        message.send(m_publisher_pub);
    }

    void xsubshell::abort_queue(const listener& l, long polling_interval)
    {
        while (true)
        {
            zmq::multipart_t wire_msg;
            bool received = wire_msg.recv(m_shell_poller, zmq::flags::dontwait);
            if (!received)
            {
                return;
            }

            auto msg = p_server->deserialize(wire_msg);
            if (msg)
            {
                l(std::move(msg.value()));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(polling_interval));
        }
    }

    zmq::multipart_t xsubshell::pop_queue(message_queue& queue)
    {
        zmq::multipart_t msg = std::move(queue.front());
        queue.pop();
        return msg;
    }
}
