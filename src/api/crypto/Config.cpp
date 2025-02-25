// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "api/crypto/Config.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/api/crypto/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace opentxs
{
constexpr auto OT_DEFAULT_ITERATION_COUNT = 65535;       // in bytes
constexpr auto OT_DEFAULT_SYMMETRIC_SALT_SIZE = 8;       // in bytes
constexpr auto OT_DEFAULT_SYMMETRIC_KEY_SIZE = 32;       // in bytes
constexpr auto OT_DEFAULT_SYMMETRIC_KEY_SIZE_MAX = 64;   // in bytes == 512 bits
constexpr auto OT_DEFAULT_SYMMETRIC_IV_SIZE = 32;        // in bytes
constexpr auto OT_DEFAULT_SYMMETRIC_BUFFER_SIZE = 4096;  // in bytes
constexpr auto OT_DEFAULT_PUBLIC_KEYSIZE = 128;      // in bytes == 4096 bits
constexpr auto OT_DEFAULT_PUBLIC_KEYSIZE_MAX = 512;  // in bytes == 1024 bits

constexpr auto OT_KEY_ITERATION_COUNT = "iteration_count";
constexpr auto OT_KEY_SYMMETRIC_SALT_SIZE = "symmetric_salt_size";
constexpr auto OT_KEY_SYMMETRIC_KEY_SIZE = "symmetric_key_size";
constexpr auto OT_KEY_SYMMETRIC_KEY_SIZE_MAX = "symmetric_key_size_max";
constexpr auto OT_KEY_SYMMETRIC_IV_SIZE = "symmetric_iv_size";
constexpr auto OT_KEY_SYMMETRIC_BUFFER_SIZE = "symmetric_buffer_size";
constexpr auto OT_KEY_PUBLIC_KEYSIZE = "public_keysize";
constexpr auto OT_KEY_PUBLIC_KEYSIZE_MAX = "public_keysize_max";
}  // namespace opentxs

namespace opentxs::factory
{
auto CryptoConfig(const api::Settings& settings) noexcept
    -> std::unique_ptr<api::crypto::Config>
{
    using ReturnType = api::crypto::imp::Config;

    return std::make_unique<ReturnType>(settings);
}
}  // namespace opentxs::factory

namespace opentxs::api::crypto
{
auto HaveSupport(opentxs::crypto::key::symmetric::Algorithm val) noexcept
    -> bool
{
    using Type = opentxs::crypto::key::symmetric::Algorithm;
    static const auto map = UnallocatedMap<Type, bool>{
        {Type::Error, false},
        {Type::ChaCha20Poly1305, true},
    };

    try {

        return map.at(val);
    } catch (...) {

        return false;
    }
}
}  // namespace opentxs::api::crypto

namespace opentxs::api::crypto::imp
{
Config::Config(const api::Settings& settings) noexcept
    : config_(settings)
{
    GetSetAll();
}

auto Config::GetSetAll() const -> bool
{
    if (!GetSetValue(
            OT_KEY_ITERATION_COUNT,
            OT_DEFAULT_ITERATION_COUNT,
            sp_nIterationCount)) {
        return false;
    }
    if (!GetSetValue(
            OT_KEY_SYMMETRIC_SALT_SIZE,
            OT_DEFAULT_SYMMETRIC_SALT_SIZE,
            sp_nSymmetricSaltSize)) {
        return false;
    }
    if (!GetSetValue(
            OT_KEY_SYMMETRIC_KEY_SIZE,
            OT_DEFAULT_SYMMETRIC_KEY_SIZE,
            sp_nSymmetricKeySize)) {
        return false;
    }
    if (!GetSetValue(
            OT_KEY_SYMMETRIC_KEY_SIZE_MAX,
            OT_DEFAULT_SYMMETRIC_KEY_SIZE_MAX,
            sp_nSymmetricKeySizeMax)) {
        return false;
    }
    if (!GetSetValue(
            OT_KEY_SYMMETRIC_IV_SIZE,
            OT_DEFAULT_SYMMETRIC_IV_SIZE,
            sp_nSymmetricIvSize)) {
        return false;
    }
    if (!GetSetValue(
            OT_KEY_SYMMETRIC_BUFFER_SIZE,
            OT_DEFAULT_SYMMETRIC_BUFFER_SIZE,
            sp_nSymmetricBufferSize)) {
        return false;
    }
    if (!GetSetValue(
            OT_KEY_PUBLIC_KEYSIZE,
            OT_DEFAULT_PUBLIC_KEYSIZE,
            sp_nPublicKeysize)) {
        return false;
    }
    if (!GetSetValue(
            OT_KEY_PUBLIC_KEYSIZE_MAX,
            OT_DEFAULT_PUBLIC_KEYSIZE_MAX,
            sp_nPublicKeysizeMax)) {
        return false;
    }

    return config_.Save();
}

auto Config::GetSetValue(
    const UnallocatedCString& strKeyName,
    const std::int32_t nDefaultValue,
    std::int32_t& out_nValue) const -> bool

{
    OT_ASSERT(false == strKeyName.empty())
    OT_ASSERT(2 < strKeyName.size())

    bool bIsNew{false};
    std::int64_t nValue{0};
    config_.CheckSet_long(
        String::Factory("crypto"),
        String::Factory(strKeyName),
        nDefaultValue,
        nValue,
        bIsNew);
    out_nValue = static_cast<std::int32_t>(nValue);

    return true;
}

auto Config::IterationCount() const -> std::uint32_t
{
    return sp_nIterationCount;
}
auto Config::SymmetricSaltSize() const -> std::uint32_t
{
    return sp_nSymmetricSaltSize;
}
auto Config::SymmetricKeySize() const -> std::uint32_t
{
    return sp_nSymmetricKeySize;
}
auto Config::SymmetricKeySizeMax() const -> std::uint32_t
{
    return sp_nSymmetricKeySizeMax;
}
auto Config::SymmetricIvSize() const -> std::uint32_t
{
    return sp_nSymmetricIvSize;
}
auto Config::SymmetricBufferSize() const -> std::uint32_t
{
    return sp_nSymmetricBufferSize;
}
auto Config::PublicKeysize() const -> std::uint32_t
{
    return sp_nPublicKeysize;
}
auto Config::PublicKeysizeMax() const -> std::uint32_t
{
    return sp_nPublicKeysizeMax;
}
}  // namespace opentxs::api::crypto::imp
