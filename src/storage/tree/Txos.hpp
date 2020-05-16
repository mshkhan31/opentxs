// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Data.hpp"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Driver;
}  // namespace storage
}  // namespace api

namespace storage
{
class Nym;
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage
{
class Txos final : public Node
{
public:
    using Coin = api::client::blockchain::Coin;
    using SerializedType = proto::StorageTxoIndex;
    using PayloadType = proto::StorageBlockchainTxo;

    auto Load(
        const Coin& id,
        std::shared_ptr<PayloadType>& output,
        const bool checking) const noexcept -> bool;
    auto LookupElement(const Data& element) const noexcept -> std::set<Coin>;
    auto LookupTxid(const std::string& txid) const noexcept -> std::set<Coin>;

    auto Delete(const Coin& id) noexcept -> bool;
    auto Store(const PayloadType& data) noexcept -> bool;

    ~Txos() = default;

private:
    friend storage::Nym;

    static const VersionNumber DefaultVersion{1};

    std::map<Coin, std::shared_ptr<PayloadType>> txos_;
    std::map<OTData, std::set<Coin>> element_index_;
    std::map<std::string, std::set<Coin>> txid_index_;

    auto save(const Lock& lock) const -> bool final;
    auto serialize() const noexcept -> SerializedType;

    void index(const Lock& lock, const PayloadType& item) noexcept;
    void init(const std::string& hash) final;

    Txos(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash) noexcept;
    Txos() = delete;
    Txos(const Txos&) = delete;
    Txos(Txos&&) = delete;
    auto operator=(const Txos&) -> Txos = delete;
    auto operator=(Txos &&) -> Txos = delete;
};
}  // namespace opentxs::storage
