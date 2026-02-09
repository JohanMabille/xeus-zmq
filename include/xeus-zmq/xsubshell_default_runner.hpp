/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XSUBSHELL_DEFAULT_RUNNER_HPP
#define XSUBSHELL_DEFAULT_RUNNER_HPP

#include "xeus-zmq.hpp"
#include "xsubshell_runner.hpp"

namespace xeus
{
    class XEUS_ZMQ_API xsubshell_default_runner : public xsubshell_runner
    {
    public:

        explicit xsubshell_default_runner(const std::string& subshell_id);
        ~xsubshell_default_runner() override = default;

    private:

        void run_impl() override;
        runner_ptr create_runner_impl(const std::string& subshell_id) const override;

        using optional_channel = std::optional<channel>;

        std::optional<xmessage> read_shell(optional_channel chan);
        std::optional<std::string> read_control(optional_channel chan);
    };
}

#endif
