// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs
{
template <typename I>
auto polarity(const I value) -> int
{
    if (0 == value) { return 0; }

    return (0 < value) ? 1 : -1;
}
}  // namespace opentxs
