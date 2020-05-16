// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "network/zeromq/socket/Request.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <type_traits>

#include "Factory.hpp"
#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Request>;

#define POLL_MILLISECONDS 10

#define OT_METHOD "opentxs::network::zeromq::socket::implementation::Request::"

namespace opentxs
{
auto Factory::RequestSocket(const network::zeromq::Context& context)
    -> network::zeromq::socket::Request*
{
    using ReturnType = network::zeromq::socket::implementation::Request;

    return new ReturnType(context);
}
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
Request::Request(const zeromq::Context& context) noexcept
    : Socket(context, SocketType::Request, Socket::Direction::Connect)
    , Client(this->get())
{
    init();
}

auto Request::clone() const noexcept -> Request*
{
    return new Request(context_);
}

auto Request::send_request(zeromq::Message& request) const noexcept
    -> Socket::SendResult
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);
    SendResult output{opentxs::SendResult::Error, Message::Factory()};
    auto& status = output.first;
    auto& reply = output.second;

    if (false == send_message(lock, request)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to deliver message.")
            .Flush();

        return output;
    }

    if (false == wait(lock)) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Receive timeout.").Flush();
        status = opentxs::SendResult::TIMEOUT;

        return output;
    }

    if (receive_message(lock, reply)) {
        status = opentxs::SendResult::VALID_REPLY;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to receive reply.")
            .Flush();
    }

    return output;
}

auto Request::SetSocksProxy(const std::string& proxy) const noexcept -> bool
{
    return set_socks_proxy(proxy);
}

auto Request::wait(const Lock& lock) const noexcept -> bool
{
    OT_ASSERT(verify_lock(lock))

    const auto start = std::chrono::system_clock::now();
    zmq_pollitem_t poll[1];
    poll[0].socket = socket_;
    poll[0].events = ZMQ_POLLIN;

    while (running_.get()) {
        std::this_thread::yield();
        const auto events = zmq_poll(poll, 1, POLL_MILLISECONDS);

        if (0 == events) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": No messages.").Flush();

            const auto now = std::chrono::system_clock::now();

            if ((now - start) > std::chrono::milliseconds(receive_timeout_)) {

                return false;
            } else {
                continue;
            }
        }

        if (0 > events) {
            const auto error = zmq_errno();
            LogOutput(OT_METHOD)(__FUNCTION__)(": Poll error: ")(
                zmq_strerror(error))(".")
                .Flush();

            return false;
        }

        return true;
    }

    return false;
}

Request::~Request() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
