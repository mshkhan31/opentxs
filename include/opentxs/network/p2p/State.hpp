// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class P2PBlockchainChainState;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::p2p
{
class OPENTXS_EXPORT State
{
public:
    auto Chain() const noexcept -> opentxs::blockchain::Type;
    auto Position() const noexcept
        -> const opentxs::blockchain::block::Position&;
    OPENTXS_NO_EXPORT auto Serialize(
        proto::P2PBlockchainChainState& dest) const noexcept -> bool;

    OPENTXS_NO_EXPORT State(
        const api::Session& api,
        const proto::P2PBlockchainChainState& serialized) noexcept(false);
    State(
        opentxs::blockchain::Type chain,
        opentxs::blockchain::block::Position position) noexcept(false);
    State() = delete;
    State(const State&) = delete;
    State(State&&) noexcept;
    auto operator=(const State&) -> State& = delete;
    auto operator=(State&&) -> State& = delete;

    ~State();

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::network::p2p
