// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>
#include <memory>

#include "opentxs/util/Container.hpp"

namespace opentxs::network
{
enum class ConnectionState : std::uint8_t {
    NOT_ESTABLISHED = 0,
    ACTIVE = 1,
    STALLED = 2
};

using DhtResults = UnallocatedVector<std::shared_ptr<UnallocatedCString>>;
using DhtDoneCallback = std::function<void(bool)>;
using DhtResultsCallback = std::function<bool(const DhtResults&)>;
}  // namespace opentxs::network
