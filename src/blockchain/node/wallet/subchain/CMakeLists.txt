# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(statemachine)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/subchain/Subchain.hpp"
    "DeterministicStateData.cpp"
    "DeterministicStateData.hpp"
    "NotificationStateData.cpp"
    "NotificationStateData.hpp"
    "ScriptForm.cpp"
    "ScriptForm.hpp"
    "SubchainStateData.cpp"
    "SubchainStateData.hpp"
)
target_link_libraries(opentxs-common PRIVATE Boost::headers)
target_include_directories(
  opentxs-common SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
)
