// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::cfilter
{
Hash::Hash() noexcept
    : FixedByteArray()
{
    static_assert(payload_size == 32u);
}

Hash::Hash(const ReadView bytes) noexcept(false)
    : FixedByteArray(bytes)
{
}

Hash::Hash(const Hash& rhs) noexcept
    : FixedByteArray(rhs)
{
}

auto Hash::operator=(const Hash& rhs) noexcept -> Hash&
{
    FixedByteArray::operator=(rhs);

    return *this;
}

Hash::~Hash() = default;
}  // namespace opentxs::blockchain::cfilter
