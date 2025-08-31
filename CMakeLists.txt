cmake_minimum_required(VERSION 3.16)
project(rdmeter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(RDMETER_ENABLE_OPENMP "Enable OpenMP" ON)
if(RDMETER_ENABLE_OPENMP)
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP enabled")
  endif()
endif()

add_library(rdmeter_lib
  src/metrics.cpp
  src/bdrate.cpp
)
target_include_directories(rdmeter_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
if(OpenMP_CXX_FOUND)
  target_link_libraries(rdmeter_lib PUBLIC OpenMP::OpenMP_CXX)
endif()

add_executable(rdmeter src/main.cpp src/yuv_reader.hpp src/threading.hpp)
target_link_libraries(rdmeter PRIVATE rdmeter_lib)

# Tests
include(FetchContent)
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(catch2)

add_executable(rdmeter_tests tests/test_bdrate.cpp tests/test_metrics.cpp)
target_link_libraries(rdmeter_tests PRIVATE rdmeter_lib Catch2::Catch2WithMain)
enable_testing()
add_test(NAME rdmeter_tests COMMAND rdmeter_tests)