// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/node/BlockOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <vector>

#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::implementation
{
const std::size_t BlockOracle::Cache::cache_limit_{16};
const std::chrono::seconds BlockOracle::Cache::download_timeout_{60};

BlockOracle::Cache::Cache(
    const api::Session& api,
    const internal::Network& node,
    const internal::BlockDatabase& db,
    const network::zeromq::socket::Publish& blockAvailable,
    const network::zeromq::socket::Publish& downloadCache,
    const blockchain::Type chain) noexcept
    : api_(api)
    , node_(node)
    , db_(db)
    , block_available_(blockAvailable)
    , cache_size_publisher_(downloadCache)
    , chain_(chain)
    , lock_()
    , pending_()
    , mem_(cache_limit_)
    , running_(true)
{
}

auto BlockOracle::Cache::DownloadQueue() const noexcept -> std::size_t
{
    auto lock = Lock{lock_};

    return pending_.size();
}

auto BlockOracle::Cache::publish(std::size_t size) const noexcept -> void
{
    auto work = api_.Network().ZeroMQ().TaggedMessage(
        WorkType::BlockchainBlockDownloadQueue);
    work->AddFrame(chain_);
    work->AddFrame(size);
    cache_size_publisher_.Send(work);
}

auto BlockOracle::Cache::publish(const block::Hash& block) const noexcept
    -> void
{
    auto work = api_.Network().ZeroMQ().TaggedMessage(
        WorkType::BlockchainBlockAvailable);
    work->AddFrame(chain_);
    work->AddFrame(block);
    block_available_.Send(work);
}

auto BlockOracle::Cache::ReceiveBlock(const zmq::Frame& in) const noexcept
    -> void
{
    ReceiveBlock(api_.Factory().BitcoinBlock(chain_, in.Bytes()));
}

auto BlockOracle::Cache::ReceiveBlock(BitcoinBlock_p in) const noexcept -> void
{
    if (false == bool(in)) {
        LogError()(OT_PRETTY_CLASS(__func__))("Invalid block").Flush();

        return;
    }

    auto lock = Lock{lock_};
    auto& block = *in;

    if (database::BlockStorage::None != db_.BlockPolicy()) {
        const auto saved = db_.BlockStore(block);

        OT_ASSERT(saved);
    }

    const auto& id = block.ID();
    auto pending = pending_.find(id);

    if (pending_.end() == pending) {
        LogVerbose()(OT_PRETTY_CLASS(__func__))(
            "Received block not in request list")
            .Flush();

        return;
    }

    auto& [time, promise, future, queued] = pending->second;
    promise.set_value(std::move(in));
    publish(id);
    LogVerbose()(OT_PRETTY_CLASS(__func__))("Cached block ")(id.asHex())
        .Flush();
    mem_.push(id, std::move(future));
    pending_.erase(pending);
    publish(pending_.size());
}

auto BlockOracle::Cache::Request(const block::Hash& block) const noexcept
    -> BitcoinBlockFuture
{
    const auto output = Request(BlockHashes{block});

    OT_ASSERT(1 == output.size());

    return output.at(0);
}

auto BlockOracle::Cache::Request(const BlockHashes& hashes) const noexcept
    -> BitcoinBlockFutures
{
    auto output = BitcoinBlockFutures{};
    output.reserve(hashes.size());
    auto download = std::map<block::pHash, BitcoinBlockFutures::iterator>{};
    auto lock = Lock{lock_};

    if (false == running_) {
        std::for_each(hashes.begin(), hashes.end(), [&](const auto&) {
            auto promise = Promise{};
            promise.set_value(nullptr);
            output.emplace_back(promise.get_future());
        });

        return output;
    }

    for (const auto& block : hashes) {
        auto found{false};

        if (auto future = mem_.find(block->Bytes()); future.valid()) {
            output.emplace_back(std::move(future));
            found = true;
        }

        if (found) { continue; }

        {
            auto it = pending_.find(block);

            if (pending_.end() != it) {
                const auto& [time, promise, future, queued] = it->second;
                output.emplace_back(future);
                found = true;
            }
        }

        if (found) { continue; }

        if (auto pBlock = db_.BlockLoadBitcoin(block); bool(pBlock)) {
            // TODO this should be checked in the block factory function
            OT_ASSERT(pBlock->ID() == block);

            auto promise = Promise{};
            promise.set_value(std::move(pBlock));
            mem_.push(OTData{block}, promise.get_future());
            output.emplace_back(mem_.find(block->Bytes()));
            found = true;
        }

        if (found) { continue; }

        output.emplace_back();
        auto it = output.begin();
        std::advance(it, output.size() - 1);
        download.emplace(block, it);
    }

    OT_ASSERT(output.size() == hashes.size());

    if (0 < download.size()) {
        auto blockList = std::vector<ReadView>{};
        std::transform(
            std::begin(download),
            std::end(download),
            std::back_inserter(blockList),
            [](const auto& in) -> auto {
                const auto& [key, value] = in;

                return key->Bytes();
            });
        LogVerbose()(OT_PRETTY_CLASS(__func__))("Downloading ")(
            blockList.size())(" blocks from peers")
            .Flush();
        const auto messageSent = node_.RequestBlocks(blockList);

        for (auto& [hash, futureOut] : download) {
            auto& [time, promise, future, queued] = pending_[hash];
            time = Clock::now();
            future = promise.get_future();
            *futureOut = future;
            queued = messageSent;
        }

        publish(pending_.size());
    }

    return output;
}

auto BlockOracle::Cache::Shutdown() noexcept -> void
{
    auto lock = Lock{lock_};

    if (running_) {
        running_ = false;
        mem_.clear();

        for (auto& [hash, item] : pending_) {
            auto& [time, promise, future, queued] = item;
            promise.set_value(nullptr);
        }

        pending_.clear();
        publish(pending_.size());
    }
}

auto BlockOracle::Cache::StateMachine() const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == running_) { return false; }

    LogVerbose()(OT_PRETTY_CLASS(__func__))(DisplayString(chain_))(
        " download queue contains ")(pending_.size())(" blocks.")
        .Flush();
    auto blockList = std::vector<ReadView>{};
    blockList.reserve(pending_.size());

    for (auto& [hash, item] : pending_) {
        auto& [time, promise, future, queued] = item;
        const auto now = Clock::now();
        namespace c = std::chrono;
        const auto elapsed = c::duration_cast<c::milliseconds>(now - time);
        const auto timeout = download_timeout_ <= elapsed;

        if (timeout || (false == queued)) {
            LogVerbose()(OT_PRETTY_CLASS(__func__))("Requesting ")(
                DisplayString(chain_))(" block ")(hash->asHex())(" from peers")
                .Flush();
            blockList.emplace_back(hash->Bytes());
            queued = true;
            time = now;
        } else {
            LogVerbose()(OT_PRETTY_CLASS(__func__))(elapsed.count())(
                " milliseconds elapsed waiting for ")(DisplayString(chain_))(
                " block ")(hash->asHex())
                .Flush();
        }
    }

    if (0 < blockList.size()) { node_.RequestBlocks(blockList); }

    return 0 < pending_.size();
}
}  // namespace opentxs::blockchain::node::implementation
