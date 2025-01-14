// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"                  // IWYU pragma: associated
#include "opentxs/blockchain/crypto/Types.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::crypto
{
enum class HDProtocol : std::uint16_t {
    Error = 0,
    BIP_32 = 32,
    BIP_44 = 44,
    BIP_49 = 49,
    BIP_84 = 84,
};
}  // namespace opentxs::blockchain::crypto
