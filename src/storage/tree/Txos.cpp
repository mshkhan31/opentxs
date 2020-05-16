// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "storage/tree/Txos.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <mutex>
#include <type_traits>
#include <utility>

#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/verify/StorageTxoIndex.hpp"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"

#define OT_METHOD "opentxs::storage::Txos::"

namespace opentxs
{
namespace storage
{
Txos::Txos(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash) noexcept
    : Node(storage, hash)
    , txos_()
    , element_index_()
    , txid_index_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(DefaultVersion);
    }
}

auto Txos::Delete(const Coin& id) noexcept -> bool
{
    Lock lock(write_lock_);
    const auto it = txos_.find(id);

    if (txos_.end() == it) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Txo not found.").Flush();

        return false;
    }

    txos_.erase(it);

    for (auto& [element, set] : element_index_) { set.erase(id); }

    for (auto& [txid, set] : txid_index_) { set.erase(id); }

    return true;
}

void Txos::index(const Lock& lock, const PayloadType& item) noexcept
{
    if (item.spent()) {
        for (const auto& txid : item.txid()) {
            txid_index_[txid].emplace(
                Coin{item.output().txid(), item.output().index()});
        }
    } else {
        for (const auto& element : item.element()) {
            element_index_[Data::Factory(element, Data::Mode::Raw)].emplace(
                Coin{item.output().txid(), item.output().index()});
        }
    }
}

void Txos::init(const std::string& hash)
{
    auto serialized = std::make_shared<SerializedType>();
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load txo index file.")
            .Flush();
        abort();
    }

    init_version(DefaultVersion, *serialized);
    Lock lock(write_lock_);

    for (const auto& txo : serialized->txo()) {
        auto [it, added] = txos_.emplace(
            Coin{txo.output().txid(), txo.output().index()},
            std::make_shared<PayloadType>(txo));

        OT_ASSERT(it->second);

        index(lock, *it->second);
    }
}

auto Txos::Load(
    const Coin& id,
    std::shared_ptr<PayloadType>& output,
    const bool checking) const noexcept -> bool
{
    Lock lock(write_lock_);
    const auto it = txos_.find(id);

    if (txos_.end() == it) {
        if (false == checking) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Txo not found.").Flush();
        }

        return false;
    }

    output = std::make_shared<PayloadType>(*it->second);

    return bool(output);
}

auto Txos::LookupElement(const Data& element) const noexcept
    -> std::set<Txos::Coin>
{
    Lock lock(write_lock_);

    try {

        return element_index_.at(element);
    } catch (...) {

        return {};
    }
}

auto Txos::LookupTxid(const std::string& txid) const noexcept
    -> std::set<Txos::Coin>
{
    Lock lock(write_lock_);

    try {

        return txid_index_.at(txid);
    } catch (...) {

        return {};
    }
}

auto Txos::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (false == verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Txos::serialize() const noexcept -> Txos::SerializedType
{
    auto output = SerializedType{};
    output.set_version(version_);

    for (const auto& [coin, txo] : txos_) {
        OT_ASSERT(txo);

        *output.add_txo() = *txo;
    }

    return output;
}

auto Txos::Store(const PayloadType& data) noexcept -> bool
{
    Lock lock(write_lock_);
    txos_[Coin{data.output().txid(), data.output().index()}] =
        std::make_shared<PayloadType>(data);
    index(lock, data);

    return save(lock);
}
}  // namespace storage
}  // namespace opentxs
