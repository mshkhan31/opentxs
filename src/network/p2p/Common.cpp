// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "opentxs/network/p2p/Types.hpp"  // IWYU pragma: associated

#include <robin_hood.h>

#include "opentxs/network/p2p/MessageType.hpp"

namespace opentxs::v1
{
using Type = network::p2p::MessageType;

auto print(Type value) noexcept -> UnallocatedCString
{
    static const auto map =
        robin_hood::unordered_flat_map<Type, UnallocatedCString>{
            {Type::sync_request, "sync request"},
            {Type::sync_ack, "sync acknowledgment"},
            {Type::sync_reply, "sync reply"},
            {Type::new_block_header, "sync push"},
            {Type::query, "sync query"},
            {Type::publish_contract, "publish contract"},
            {Type::publish_ack, "publish acknowledgment"},
            {Type::contract_query, "contract query"},
            {Type::contract, "contract reply"},
        };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}
}  // namespace opentxs::v1
