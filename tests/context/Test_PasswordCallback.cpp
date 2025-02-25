// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/otx/common/Message.hpp"
#include "ottest/Basic.hpp"

namespace ottest
{
ot::UnallocatedCString profile_id_{};
ot::UnallocatedCString nym_id_{};
ot::UnallocatedCString server_id_{};

#define TEST_PASSWORD "blah foo blah foo blah"
#define TEST_DIFF_PASSWORD "time keeps on slippin slippin slippin"

class TestCallback : public opentxs::PasswordCallback
{
    ot::UnallocatedCString password_;

public:
    void runOne(
        const char* szDisplay,
        opentxs::Secret& theOutput,
        const ot::UnallocatedCString& key) const override
    {
        theOutput.AssignText(password_);
    }

    void runTwo(
        const char* szDisplay,
        opentxs::Secret& theOutput,
        const ot::UnallocatedCString& key) const override
    {
        theOutput.AssignText(password_);
    }

    void SetPassword(const ot::UnallocatedCString& password)
    {
        password_ = password;
    }

    TestCallback();
};

TestCallback::TestCallback()
    : opentxs::PasswordCallback()
    , password_()
{
}

TEST(PasswordCallback, create)
{
    opentxs::PasswordCaller caller;
    TestCallback callback;
    callback.SetPassword(TEST_PASSWORD);
    caller.SetCallback(&callback);

    const auto& otx = ot::InitContext(Args(true), &caller);
    profile_id_ = otx.ProfileId();
    const auto& client = otx.StartClientSession(0);
    const auto reason = client.Factory().PasswordPrompt(__func__);
    const auto nym = client.Wallet().Nym(reason);

    ASSERT_TRUE(nym);
    nym_id_ = nym->ID().str();
    EXPECT_FALSE(nym_id_.empty());

    ot::Cleanup();
}

TEST(PasswordCallback, load)
{
    opentxs::PasswordCaller caller;
    TestCallback callback;
    callback.SetPassword(TEST_PASSWORD);
    caller.SetCallback(&callback);

    const auto& otx = ot::InitContext(Args(true), &caller);
    const auto profile_id = otx.ProfileId();
    EXPECT_EQ(profile_id, profile_id_);

    const auto& client = otx.StartClientSession(0);
    const auto nym_identifier{opentxs::identifier::Nym::Factory(nym_id_)};
    const auto nym = client.Wallet().Nym(nym_identifier);

    ASSERT_TRUE(nym);
    const auto nym_id = nym->ID().str();
    EXPECT_FALSE(nym_id.empty());
    EXPECT_EQ(nym_id, nym_id_);

    // Have the Nym sign something here, which should succeed.

    auto reason = client.Factory().PasswordPrompt(__func__);
    auto message{client.Factory().InternalSession().Message()};

    const auto signed_success = message->SignContract(*nym, reason);
    ASSERT_TRUE(signed_success);

    ot::Cleanup();
}

TEST(PasswordCallback, wrongpw)
{
    opentxs::PasswordCaller caller;
    TestCallback callback;
    callback.SetPassword(TEST_DIFF_PASSWORD);
    caller.SetCallback(&callback);

    const auto& otx = ot::InitContext(Args(true), &caller);
    const auto& client = otx.StartClientSession(0);
    const auto nym_identifier{opentxs::identifier::Nym::Factory(nym_id_)};
    const auto nym = client.Wallet().Nym(nym_identifier);

    ASSERT_TRUE(nym);

    // Have the Nym sign something here, which should fail since
    // we deliberately used the wrong password.

    auto reason = client.Factory().PasswordPrompt(__func__);
    auto message{client.Factory().InternalSession().Message()};

    const auto signed_success = message->SignContract(*nym, reason);
    ASSERT_FALSE(signed_success);

    ot::Cleanup();
}
}  // namespace ottest
