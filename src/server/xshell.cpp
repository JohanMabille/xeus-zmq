/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <algorithm>
#include <chrono>
#include <thread>

#include "xserver_zmq_split_impl.hpp"
#include "xshell.hpp"
#include "../common/xmiddleware_impl.hpp"

namespace xeus
{
    xshell::xshell(zmq::context_t& context,
                   const std::string& transport,
                   const std::string& ip,
                   const std::string& shell_port,
                   const std::string& stdin_port)
        : m_shell(context, zmq::socket_type::router)
        , m_stdin(context, zmq::socket_type::router)
        , m_controller(context, zmq::socket_type::rep)
        , p_context(&context)
    {
        init_socket(m_shell, transport, ip, shell_port);
        init_socket(m_stdin, transport, ip, stdin_port);

        m_controller.set(zmq::sockopt::linger, get_socket_linger());
        m_controller.bind(get_controller_end_point("shell"));

        m_poll_items =
        {
            { m_shell, 0, ZMQ_POLLIN, 0},
            { m_stdin, 0, ZMQ_POLLIN, 0},
            { m_controller, 0, ZMQ_POLLIN, 0}
        };

        add_subshell("");
    }

    std::string xshell::get_shell_port() const
    {
        return get_socket_port(m_shell);
    }

    std::string xshell::get_stdin_port() const
    {
        return get_socket_port(m_stdin);
    }

    void run()
    {
        bool stop_required = false;
        while (!stop_required)
        {
            zmq::poll(m_pollitem_list.data(), m_pollitem_list.size(), -1);

            if (m_pollitem_list[0].revents & ZMQ_POLLIN)
            {
                dispatch(m_shell, get_shell_routing_key());
            }
            else if (m_pollitem_list[1].revents & ZMQ_POLLIN)
            {
                dispatch(m_stdin, get_stdin_routing_key());
            }
            else if(m_pollitem_list[2].revents & ZMQ_POLLIN)
            {
                stop_required = dispatch_controller();
            }
            else
            {
                for (std::size_t i = SUBSHELL_OFFSET; i < items.size(); ++i)
                {
                    if (m_pollitem_list[i].revents & ZMQ_POLLIN)
                    {
                        dispatch_subshell(i - SUBSHELL_OFFSET);
                        break;
                    }
                }
            }
        }
    }

    void xshell::dispatch(zmq::socket_t& socket, std::string routing_key)
    {
        zmq::multipart_t wire_msg;
        wire_msg.recv(socket);
        std::string subshell_id = get_subshell_id(wire_msg);
        wire_msg.push(routing_key);
        auto subshell_iter = find_subshell(subshell_id);
        if (subshell_iter != m_subshell_list.cend())
        {
            wire_msg.send(subshell_iter->socket);
        }
        // TODO: handle non existing subshell
    }

    bool xshell::dispatch_controller()
    {
        zmq::multipart_t wire_msg;
        wire_msg.recv(m_controller);
        std::string msg = wire_msg.peekstr(0u);
        if (msg == "stop")
        {
            std::for_each(m_subshell_list.begin(), m_subshell_list.end(), [](const auto& subshell)
            {
                zmq::multipart_t msg(std::string("stop"));
                msg.push(get_controller_routing_key());
                msg.send(subshell.socket);
            });
            wire_msg.send(socket);
            return true;
        }
        else if (msg == "add_subshell")
        {
            std::string subshell_id = wire_msg.peekstr(1u);
            bool res = add_subshell(subshell_id);
            std::string status = res ? "success" : "error";
            zmq::multipart_t rep(std::move(status));
            rep.send(m_controller);
        }
        else if (msg == "remove_subshell")
        {
            std::string subshell_id = wire_msg.peekstr(1u);
            bool res = remove_subshell(subshell_id);
            std::string status = res ? "success" : "error";
            zmq::multipart_t rep(std::move(status));
            rep.send(m_controller);
        }
        else
        {
            // Messages received on the controller should be forwarded to the
            // parent subshell.
            wire_msg.send(m_subshell_list.front().socket);
        }
        return false;
    }

    void xshell::dispatch_subshell(std::size_t i)
    {
        zmq::multipart_t wire_msg;
        wire_msg.recv(m_subshell_list[i].socket);
        std::string routing_key = wire_msg.popstr();
        if (routing_key == get_shell_rounting_key())
        {
            wire_msg.send(m_shell);
        }
        else if (routing_key == get_stdin_routing_key())
        {
            wire_msg.send(m_stdin);
        }
        else
        {
            wire_msg.send(m_controller);
        }
    }

    bool xshell::add_subshell(const std::string& id)
    {
        if (auto iter = find_subshell(id); iter != m_subshell_list.cend())
        {
            return false;
        }

        subshell_t subshell{id, zmq::socket_t(*p_context, zmq::socket_type::pair)};
        subshell.socket.set(zmq::sockopt::linger, get_socket_linger());
        subshell.socket.bind(get_subshell_end_point(id));
        m_subshell_list.push_back(std::move(subshell));
        m_pollitem_list.push_back({m_subshell_list.back().socket, 0, ZMQ_POLLIN, 0});
        return true;
    }

    bool xshell::remove_subshell(const std::string& id)
    {
        if (id == "")
        {
            return false;
        }

        auto iter = find_subshell(id);
        if (iter == m_subshell_list.cend())
        {
            return false;
        }

        auto idx = std::distance(m_subshell_list.cbegin(), iter);
        m_subshell_list.erase(iter);
        m_pollitem_list.erase(std::advance(m_pollitem_list.cbegin(), idx));
        return true;
    }

    auto xshell::find_subshell(const std::string& id) const -> subshell_iterator
    {
        return std::find_if(
            m_subshell_list.cbegin(),
            m_subshell_list.cend(),
            [&id](const subshell_t& sub) { return sub.name == id; }
        );
    }
}

