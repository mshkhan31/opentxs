# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-common PRIVATE "Multiplex.cpp" "Multiplex.hpp")

if(FS_EXPORT)
  target_sources(opentxs-common PRIVATE "fs.cpp")
  target_link_libraries(opentxs-common PRIVATE Boost::headers)
  target_link_libraries(opentxs PUBLIC Boost::filesystem)
else()
  target_sources(opentxs-common PRIVATE "nofs.cpp")
endif()

if(SQLITE_EXPORT)
  target_sources(opentxs-common PRIVATE "sqlite.cpp")
  target_link_libraries(opentxs-common PRIVATE SQLite::SQLite3)
  target_link_libraries(opentxs PUBLIC SQLite::SQLite3)
else()
  target_sources(opentxs-common PRIVATE "nosqlite.cpp")
endif()

if(LMDB_EXPORT)
  target_sources(opentxs-common PRIVATE "lmdb.cpp")
  target_link_libraries(opentxs-common PRIVATE lmdb)
  target_link_libraries(opentxs PUBLIC lmdb)
else()
  target_sources(opentxs-common PRIVATE "nolmdb.cpp")
endif()
