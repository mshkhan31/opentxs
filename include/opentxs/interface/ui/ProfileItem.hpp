// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "ListRow.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class ProfileItem;
}  // namespace ui

using OTUIProfileItem = SharedPimpl<ui::ProfileItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT ProfileItem : virtual public ListRow
{
public:
    virtual auto ClaimID() const noexcept -> UnallocatedCString = 0;
    virtual auto Delete() const noexcept -> bool = 0;
    virtual auto IsActive() const noexcept -> bool = 0;
    virtual auto IsPrimary() const noexcept -> bool = 0;
    virtual auto SetActive(const bool& active) const noexcept -> bool = 0;
    virtual auto SetPrimary(const bool& primary) const noexcept -> bool = 0;
    virtual auto SetValue(const UnallocatedCString& value) const noexcept
        -> bool = 0;
    virtual auto Value() const noexcept -> UnallocatedCString = 0;

    ~ProfileItem() override = default;

protected:
    ProfileItem() noexcept = default;

private:
    ProfileItem(const ProfileItem&) = delete;
    ProfileItem(ProfileItem&&) = delete;
    auto operator=(const ProfileItem&) -> ProfileItem& = delete;
    auto operator=(ProfileItem&&) -> ProfileItem& = delete;
};
}  // namespace opentxs::ui
