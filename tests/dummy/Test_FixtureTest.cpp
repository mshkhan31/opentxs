// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

// fixture at global namespace
class FixtureClassAtGlobalNameSpace : public ::testing::Test
{
private:
    bool mBool;

public:
    FixtureClassAtGlobalNameSpace()
        : mBool(false)
    {
    }
    ~FixtureClassAtGlobalNameSpace() override = default;

    virtual auto getBool() const -> bool { return mBool; }

    void SetUp() override { mBool = true; }

    void TearDown() override { mBool = false; }
};

TEST_F(FixtureClassAtGlobalNameSpace, simpleFixture) { EXPECT_TRUE(getBool()); }

// fixture at unnamed namespace
namespace
{

class FixtureClassAtUnnamedNamespace : public ::testing::Test
{
private:
    bool mBool;

public:
    FixtureClassAtUnnamedNamespace()
        : mBool(false)
    {
    }
    ~FixtureClassAtUnnamedNamespace() override = default;

    virtual auto getBool() const -> bool { return mBool; }

    void SetUp() override { mBool = true; }

    void TearDown() override { mBool = false; }
};

TEST_F(FixtureClassAtUnnamedNamespace, simpleFixture)
{
    EXPECT_TRUE(getBool());
}
}  // namespace

// fixture at named namespace
namespace ottest::DummyTest
{
class FixtureClassInsideNamedNamespace : public ::testing::Test
{
private:
    bool mBool;

public:
    FixtureClassInsideNamedNamespace()
        : mBool(false)
    {
    }
    ~FixtureClassInsideNamedNamespace() override = default;

    virtual auto getBool() const -> bool { return mBool; }

    void SetUp() override { mBool = true; }

    void TearDown() override { mBool = false; }
};

TEST_F(FixtureClassInsideNamedNamespace, simpleFixture)
{
    EXPECT_TRUE(this->getBool());
}
}  // namespace ottest::DummyTest
