// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <memory>

#include "internal/blockchain/node/Mempool.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Transaction;
}  // namespace internal

class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace database
{
class Wallet;
}  // namespace database
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class Mempool final : public internal::Mempool
{
public:
    auto Dump() const noexcept -> UnallocatedSet<UnallocatedCString> final;
    auto Query(ReadView txid) const noexcept
        -> std::shared_ptr<const bitcoin::block::Transaction> final;
    auto Submit(ReadView txid) const noexcept -> bool final;
    auto Submit(const UnallocatedVector<ReadView>& txids) const noexcept
        -> UnallocatedVector<bool> final;
    auto Submit(std::unique_ptr<const bitcoin::block::Transaction> tx)
        const noexcept -> void final;

    auto Heartbeat() noexcept -> void final;

    Mempool(
        const api::crypto::Blockchain& crypto,
        database::Wallet& db,
        const network::zeromq::socket::Publish& socket,
        const Type chain) noexcept;
    Mempool() = delete;
    Mempool(const Mempool&) = delete;
    Mempool(Mempool&&) = delete;
    auto operator=(const Mempool&) -> Mempool& = delete;
    auto operator=(Mempool&&) -> Mempool& = delete;

    ~Mempool() final;

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node
