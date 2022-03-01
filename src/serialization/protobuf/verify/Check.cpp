// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "serialization/protobuf/verify/Check.hpp"  // IWYU pragma: associated

#ifdef USE_ANDROID_LOG
#include <android/log.h>
#else
#include <iostream>
#endif
#include "internal/serialization/protobuf/Check.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::v1::proto
{
void PrintErrorMessage(const char* proto, const char* error) noexcept
{
    std::stringstream out{};
    out << error << " for " << proto << std::endl;
    WriteLogMessage(out);
}

void PrintErrorMessage(
    const char* proto,
    const unsigned int version,
    const char* error) noexcept
{
    std::stringstream out{};
    out << "Verify version " << version << " " << proto << " failed: " << error
        << std::endl;
    WriteLogMessage(out);
}

void PrintErrorMessage(
    const char* proto,
    const unsigned int version,
    const UnallocatedCString& error) noexcept
{
    std::stringstream out{};
    out << "Verify version " << version << " " << proto << " failed: " << error
        << std::endl;
    WriteLogMessage(out);
}

void PrintErrorMessage(
    const char* proto,
    const unsigned int version,
    const char* error,
    const long long int value) noexcept
{
    std::stringstream out{};
    out << "Verify version " << version << " " << proto << " failed: " << error
        << "(" << value << ")" << std::endl;
    WriteLogMessage(out);
}

void PrintErrorMessage(
    const char* proto,
    const unsigned int version,
    const char* error,
    const UnallocatedCString& value) noexcept
{
    std::stringstream out{};
    out << "Verify version " << version << " " << proto << " failed: " << error
        << "(" << value << ")" << std::endl;
    WriteLogMessage(out);
}

void WriteLogMessage(const std::stringstream& out) noexcept
{
#ifdef USE_ANDROID_LOG
    __android_log_write(ANDROID_LOG_INFO, "opentxs-proto", out.str().c_str());
#else
    std::cerr << out.str();
#endif
}
}  // namespace opentxs::v1::proto
