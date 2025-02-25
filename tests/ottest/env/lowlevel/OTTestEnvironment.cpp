// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/env/OTTestEnvironment.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "ottest/Basic.hpp"

namespace ottest
{
OTTestEnvironment::OTTestEnvironment() noexcept = default;

auto OTTestEnvironment::SetUp() -> void {}

auto OTTestEnvironment::TearDown() -> void
{
    ot::Cleanup();
    WipeHome();
}

OTTestEnvironment::~OTTestEnvironment() = default;
}  // namespace ottest
