// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include "blockchain/block/Header.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

class Factory;
}  // namespace opentxs

namespace be = boost::endian;

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Header final : virtual public internal::Header,
                     public block::implementation::Header
{
public:
    struct BitcoinFormat {
        be::little_int32_buf_t version_;
        std::array<char, 32> previous_;
        std::array<char, 32> merkle_;
        be::little_uint32_buf_t time_;
        be::little_int32_buf_t nbits_;
        be::little_uint32_buf_t nonce_;

        BitcoinFormat(
            const std::int32_t version,
            const std::string& previous,
            const std::string& merkle,
            const std::uint32_t time,
            const std::int32_t nbits,
            const std::uint32_t nonce) noexcept(false);
        BitcoinFormat() noexcept;
    };

    static const VersionNumber local_data_version_;
    static const VersionNumber subversion_default_;

    static auto calculate_hash(
        const api::client::Manager& api,
        const blockchain::Type chain,
        const ReadView serialized) -> block::pHash;

    auto clone() const noexcept -> std::unique_ptr<block::Header> final;
    auto Encode() const noexcept -> OTData final;
    auto MerkleRoot() const noexcept -> const block::Hash& final
    {
        return merkle_root_;
    }
    auto Nonce() const noexcept -> std::uint32_t final { return nonce_; }
    auto Serialize() const noexcept -> SerializedType final;
    auto Serialize(const AllocateOutput destination) const noexcept
        -> bool final;
    auto Target() const noexcept -> OTNumericHash final;
    auto Timestamp() const noexcept -> Time final { return timestamp_; }
    auto Version() const noexcept -> std::uint32_t final
    {
        return block_version_;
    }

    Header(
        const api::client::Manager& api,
        const blockchain::Type chain,
        const VersionNumber subversion,
        const block::Hash& hash,
        const std::int32_t version,
        const block::Hash& previous,
        const block::Hash& merkle,
        const Time timestamp,
        const std::int32_t nbits,
        const std::uint32_t nonce) noexcept;
    Header(
        const api::client::Manager& api,
        const blockchain::Type chain,
        const block::Hash& hash,
        const block::Hash& parentHash,
        const block::Height height) noexcept;
    Header(
        const api::client::Manager& api,
        const SerializedType& serialized) noexcept;

    ~Header() final = default;

private:
    using ot_super = block::implementation::Header;

    const VersionNumber subversion_;
    const std::int32_t block_version_;
    const OTData merkle_root_;
    const Time timestamp_;
    const std::int32_t nbits_;
    const std::uint32_t nonce_;

    static auto calculate_hash(
        const api::client::Manager& api,
        const SerializedType& serialized) -> block::pHash;
    static auto calculate_work(const std::int32_t nbits) -> OTWork;

    Header() = delete;
    Header(const Header& rhs) noexcept;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header &&) -> Header& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
