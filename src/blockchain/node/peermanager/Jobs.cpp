// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/peermanager/PeerManager.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::implementation
{
PeerManager::Jobs::Jobs(const api::Session& api) noexcept
    : zmq_(api.Network().ZeroMQ())
    , getheaders_(zmq_.PushSocket(zmq::socket::Direction::Bind))
    , getcfheaders_(zmq_.PublishSocket())
    , getcfilters_(zmq_.PublishSocket())
    , getblocks_(zmq_.PublishSocket())
    , heartbeat_(zmq_.PublishSocket())
    , getblock_(zmq_.PushSocket(zmq::socket::Direction::Bind))
    , broadcast_transaction_(zmq_.PushSocket(zmq::socket::Direction::Bind))
    , broadcast_block_(zmq_.PublishSocket())
    , endpoint_map_([&] {
        auto map = EndpointMap{};
        listen(map, PeerManagerJobs::Getheaders, getheaders_);
        listen(map, PeerManagerJobs::JobAvailableCfheaders, getcfheaders_);
        listen(map, PeerManagerJobs::JobAvailableCfilters, getcfilters_);
        listen(map, PeerManagerJobs::JobAvailableBlock, getblocks_);
        listen(map, PeerManagerJobs::Heartbeat, heartbeat_);
        listen(map, PeerManagerJobs::Getblock, getblock_);
        listen(
            map, PeerManagerJobs::BroadcastTransaction, broadcast_transaction_);
        listen(map, PeerManagerJobs::BroadcastBlock, broadcast_block_);

        return map;
    }())
    , socket_map_({
          {PeerManagerJobs::Getheaders, &getheaders_.get()},
          {PeerManagerJobs::JobAvailableCfheaders, &getcfheaders_.get()},
          {PeerManagerJobs::JobAvailableBlock, &getcfilters_.get()},
          {PeerManagerJobs::JobAvailableCfilters, &getblocks_.get()},
          {PeerManagerJobs::Heartbeat, &heartbeat_.get()},
          {PeerManagerJobs::Getblock, &getblock_.get()},
          {PeerManagerJobs::BroadcastTransaction,
           &broadcast_transaction_.get()},
          {PeerManagerJobs::BroadcastBlock, &broadcast_block_.get()},
      })
{
    // WARNING if any publish sockets are converted to push sockets or vice
    // versa then blockchain::p2p::implementation::Peer::subscribe() must also
    // be updated
}

auto PeerManager::Jobs::Dispatch(const PeerManagerJobs type) noexcept -> void
{
    Dispatch(Work(type));
}

auto PeerManager::Jobs::Dispatch(zmq::Message&& work) noexcept -> void
{
    const auto body = work.Body();

    OT_ASSERT(0 < body.size());

    const auto task = [&] {
        try {

            return body.at(0).as<PeerManagerJobs>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    socket_map_.at(task)->Send(std::move(work));
}

auto PeerManager::Jobs::Endpoint(const PeerManagerJobs type) const noexcept
    -> UnallocatedCString
{
    try {

        return endpoint_map_.at(type);
    } catch (...) {

        return {};
    }
}

auto PeerManager::Jobs::listen(
    EndpointMap& map,
    const PeerManagerJobs type,
    const zmq::socket::Sender& socket) noexcept -> void
{
    auto [it, added] =
        map.emplace(type, network::zeromq::MakeArbitraryInproc());

    OT_ASSERT(added);

    const auto listen = socket.Start(it->second);

    OT_ASSERT(listen);
}

auto PeerManager::Jobs::Shutdown() noexcept -> void
{
    for (auto [type, socket] : socket_map_) { socket->Close(); }
}

auto PeerManager::Jobs::Work(const PeerManagerJobs task) const noexcept
    -> network::zeromq::Message
{
    return network::zeromq::tagged_message(task);
}
}  // namespace opentxs::blockchain::node::implementation
