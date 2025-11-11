/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SHELL_HPP
#define XEUS_SHELL_HPP

#include <optional>
#include <string>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "xeus/xmessage.hpp"
#include "xeus/xserver.hpp"

#include "xeus-zmq/xmiddleware.hpp"

namespace xeus
{
    class xshell
    {
    public:

        xshell(zmq::context_t& context,
               const std::string& transport,
               const std::string& ip,
               const std::string& shell_port,
               const std::string& stdin_port);
 
        std::string get_shell_port() const;
        std::string get_stdin_port() const;

        void run();

    private:

        struct subshell_t
        {
            std::string name;
            zmq::socket_t socket;
        };
        using subshell_list_t = std::vector<subshell_t>;
        using subshell_iterator = subshell_list_t::const_iterator;
        using polliter_list_t = std::vector<zmq::pollitem_t>;

        bool add_subshell(const std::string& id);
        bool remove_subshell(const std::string& id);
        subshell_iterator find_subshell(const std::string& id) const;

        zmq::socket_t m_shell;
        zmq::socket_t m_stdin;
        zmq::socket_t m_controller;
        subshell_list_t m_subshell_list;
        pollitem_list_t m_pollitem_list;
        zmq::context_t* p_context;

        static constexpr SUBSHELL_OFFSET = 3;
    };
 }

#endif

