cmake_minimum_required(VERSION 3.0.0)

project(libkperfdata VERSION 0.1.0)

set(CMAKE_C_STANDARD 99)

# library: libkperfdata
include_directories(${PROJECT_SOURCE_DIR}/include)
set(KPERFDATA_HEADERS
  ${PROJECT_SOURCE_DIR}/include/kperfdata/kperfdata.h
)
set(KPERFDATA_SOURCES
  ${PROJECT_SOURCE_DIR}/src/kperfdata.c
)
add_library(${PROJECT_NAME} ${KPERFDATA_HEADERS} ${KPERFDATA_SOURCES})

# test
if(BUILD_TESTING)
  enable_language(CXX)

  # GoogleTest requires at least C++14
  set(CMAKE_CXX_STANDARD 14)

  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_executable(
    ${PROJECT_NAME}_test
    test/kperfdata_test.cpp
  )
  target_link_libraries(
    ${PROJECT_NAME}_test
    ${PROJECT_NAME}
    gtest_main
  )
  include(GoogleTest)
  gtest_discover_tests(${PROJECT_NAME}_test)
endif()