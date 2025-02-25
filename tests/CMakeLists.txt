# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(ottest OBJECT "")
add_library(ottest-basic OBJECT "")
add_library(ottest-lowlevel OBJECT "")
add_library(ottest-pch OBJECT "pch.cpp")
target_link_libraries(ottest-pch PRIVATE ottest)
set_target_properties(
  ottest-pch
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
    POSITION_INDEPENDENT_CODE 1
    UNITY_BUILD OFF
)

if(OT_PCH)
  target_precompile_headers(
    ottest-pch
    PUBLIC
    <gmock/gmock.h>
    <gtest/gtest.h>
    <opentxs/opentxs.hpp>
  )
endif()

function(
  add_opentx_test_target
  target_name
  cxx-sources
)
  add_executable(${target_name} ${cxx-sources} $<TARGET_OBJECTS:ottest>)
  target_link_libraries(${target_name} PRIVATE ottest)
  set_target_properties(
    ${target_name}
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
      POSITION_INDEPENDENT_CODE 1
      UNITY_BUILD OFF
  )

  if(WIN32)
    set_target_properties(${target_name} PROPERTIES LINK_OPTIONS /STACK:2097152)
  endif()

  if(OT_PCH)
    target_precompile_headers(
      ${target_name}
      REUSE_FROM
      ottest-pch
    )
  endif()

  add_test(
    ${target_name}
    ${PROJECT_BINARY_DIR}/tests/${target_name}
    --gtest_output=xml:gtestresults.xml
  )
endfunction()

function(
  add_opentx_test
  target_name
  file_name
)
  set(cxx-sources "${file_name}" $<TARGET_OBJECTS:ottest-basic>)
  add_opentx_test_target("${target_name}" "${cxx-sources}")
endfunction()

function(
  add_opentx_low_level_test
  target_name
  file_name
)
  set(cxx-sources "${file_name}" $<TARGET_OBJECTS:ottest-lowlevel>)
  add_opentx_test_target("${target_name}" "${cxx-sources}")
endfunction()

add_subdirectory(blind)
add_subdirectory(blockchain)
add_subdirectory(client)
add_subdirectory(contact)
add_subdirectory(context)
add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(dummy)
add_subdirectory(identity)
add_subdirectory(integration)
add_subdirectory(network/zeromq)
add_subdirectory(ottest)
add_subdirectory(otx)
add_subdirectory(paymentcode)
add_subdirectory(rpc)
add_subdirectory(ui)
