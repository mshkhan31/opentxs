// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "api/crypto/base/Crypto.hpp"  // IWYU pragma: associated

#include "internal/util/LogMacros.hpp"

namespace opentxs::v1::api::imp
{
auto Crypto::Init_Libsecp256k1() noexcept -> void {}

auto Crypto::Libsecp256k1() const noexcept -> const opentxs::crypto::Secp256k1&
{
    OT_FAIL;
}
}  // namespace opentxs::v1::api::imp
