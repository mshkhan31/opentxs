// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "util/storage/tree/Servers.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/ServerContract.hpp"
#include "internal/serialization/protobuf/verify/StorageServers.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "serialization/protobuf/ServerContract.pb.h"
#include "serialization/protobuf/StorageItemHash.pb.h"
#include "serialization/protobuf/StorageServers.pb.h"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage
{
Servers::Servers(const Driver& storage, const UnallocatedCString& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(2);
    }
}

auto Servers::Alias(const UnallocatedCString& id) const -> UnallocatedCString
{
    return get_alias(id);
}

auto Servers::Delete(const UnallocatedCString& id) -> bool
{
    return delete_item(id);
}

void Servers::init(const UnallocatedCString& hash)
{
    std::shared_ptr<proto::StorageServers> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __func__ << ": Failed to load servers index file."
                  << std::endl;
        abort();
    }

    init_version(2, *serialized);

    for (const auto& it : serialized->server()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

auto Servers::Load(
    const UnallocatedCString& id,
    std::shared_ptr<proto::ServerContract>& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    return load_proto<proto::ServerContract>(id, output, alias, checking);
}

void Servers::Map(ServerLambda lambda) const
{
    map<proto::ServerContract>(lambda);
}

auto Servers::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Servers::serialize() const -> proto::StorageServers
{
    proto::StorageServers serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_server());
        }
    }

    return serialized;
}

auto Servers::SetAlias(
    const UnallocatedCString& id,
    const UnallocatedCString& alias) -> bool
{
    return set_alias(id, alias);
}

auto Servers::Store(
    const proto::ServerContract& data,
    const UnallocatedCString& alias,
    UnallocatedCString& plaintext) -> bool
{
    return store_proto(data, data.id(), alias, plaintext);
}
}  // namespace opentxs::storage
