// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>

#include "internal/blockchain/database/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Position;
}  // namespace block

namespace database
{
namespace wallet
{
class Proposal;
class SubchainData;
}  // namespace wallet
}  // namespace database

namespace node
{
namespace internal
{
struct SpendPolicy;
}  // namespace internal
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

extern "C" {
using MDB_txn = struct MDB_txn;
}

namespace opentxs::blockchain::database::wallet
{
using AccountID = Identifier;
using SubchainID = Identifier;
using Parent = database::Wallet;
using NodeID = Parent::NodeID;
using BatchedMatches = Parent::BatchedMatches;
using UTXO = Parent::UTXO;
using TXOs = Parent::TXOs;

class Output
{
public:
    auto GetBalance() const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner) const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner, const NodeID& node)
        const noexcept -> Balance;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance;
    auto GetOutputs(node::TxoState type, alloc::Resource* alloc) const noexcept
        -> Vector<UTXO>;
    auto GetOutputs(
        const identifier::Nym& owner,
        node::TxoState type,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>;
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        node::TxoState type,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>;
    auto GetOutputs(
        const crypto::Key& key,
        node::TxoState type,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO>;
    auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<node::TxoTag>;
    auto GetPosition() const noexcept -> block::Position;
    auto GetTransactions() const noexcept -> UnallocatedVector<block::pTxid>;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::pTxid>;
    auto GetUnconfirmedTransactions() const noexcept
        -> UnallocatedSet<block::pTxid>;
    auto GetUnspentOutputs(alloc::Resource* alloc) const noexcept
        -> Vector<UTXO>;
    auto GetUnspentOutputs(const NodeID& balanceNode, alloc::Resource* alloc)
        const noexcept -> Vector<UTXO>;
    auto GetWalletHeight() const noexcept -> block::Height;
    auto PublishBalance() const noexcept -> void;

    auto AddConfirmedTransactions(
        const NodeID& account,
        const SubchainID& index,
        BatchedMatches&& transactions,
        TXOs& txoCreated,
        TXOs& txoConsumed) noexcept -> bool;
    auto AddMempoolTransaction(
        const AccountID& account,
        const SubchainID& subchain,
        const Vector<std::uint32_t> outputIndices,
        const bitcoin::block::Transaction& transaction,
        TXOs& txoConsumed) const noexcept -> bool;
    auto AddOutgoingTransaction(
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const bitcoin::block::Transaction& transaction) noexcept -> bool;
    auto AdvanceTo(const block::Position& pos) noexcept -> bool;
    auto CancelProposal(const Identifier& id) noexcept -> bool;
    auto FinalizeReorg(MDB_txn* tx, const block::Position& pos) noexcept
        -> bool;
    auto ReserveUTXO(
        const identifier::Nym& spender,
        const Identifier& proposal,
        node::internal::SpendPolicy& policy) noexcept -> std::optional<UTXO>;
    auto StartReorg(
        MDB_txn* tx,
        const SubchainID& subchain,
        const block::Position& position) noexcept -> bool;

    Output(
        const api::Session& api,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type chain,
        const wallet::SubchainData& subchains,
        wallet::Proposal& proposals) noexcept;
    Output() = delete;
    Output(const Output&) = delete;
    auto operator=(const Output&) -> Output& = delete;
    auto operator=(Output&&) -> Output& = delete;

    ~Output();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::wallet
