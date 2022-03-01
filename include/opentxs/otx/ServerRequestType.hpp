// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"    // IWYU pragma: associated
#include "opentxs/otx/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs::v1::otx
{
enum class ServerRequestType : std::uint8_t {
    Error = 0,
    Activate = 1,
};
}  // namespace opentxs::v1::otx
