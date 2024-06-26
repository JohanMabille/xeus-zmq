/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_CONTROL_HPP
#define XEUS_CONTROL_HPP

#include <string>

#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "xzmq_messenger.hpp"

#include "xeus/xmessage.hpp"

#include "xeus-zmq/xmiddleware.hpp"

namespace xeus
{
    class xserver_zmq_split_impl;

    class xcontrol
    {
    public:

        using listener = std::function<void(zmq::multipart_t&)>;

        xcontrol(zmq::context_t& context,
                 const std::string& transport,
                 const std::string& ip,
                 const std::string& control_port,
                 xserver_zmq_split_impl* server);

        std::string get_port() const;
        fd_t get_fd() const;

        void connect_messenger();
        void stop_channels();
        xcontrol_messenger& get_messenger();
        
        std::optional<xmessage> read_control(int flags);

        void send_control(zmq::multipart_t& message);
        void publish(zmq::multipart_t& message);

    private:

        zmq::socket_t m_control;
        zmq::socket_t m_publisher_pub;
        // Internal sockets for controlling other threads
        xzmq_messenger m_messenger;
        xserver_zmq_split_impl* p_server;
    };
}

#endif

