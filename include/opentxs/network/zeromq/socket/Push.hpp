// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/zeromq/curve/Client.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
namespace socket
{
class Push;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQPushSocket = Pimpl<network::zeromq::socket::Push>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket
{
class OPENTXS_EXPORT Push : virtual public curve::Client, virtual public Sender
{
public:
    Push(const Push&) = delete;
    Push(Push&&) = delete;
    auto operator=(const Push&) -> Push& = delete;
    auto operator=(Push&&) -> Push& = delete;

    ~Push() override = default;

protected:
    Push() noexcept = default;

private:
    friend OTZMQPushSocket;

    virtual auto clone() const noexcept -> Push* = 0;
};
}  // namespace opentxs::network::zeromq::socket
