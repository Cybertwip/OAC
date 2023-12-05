CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(SmallFBX)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE PATH "" FORCE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -std=c++17")

add_subdirectory(SmallFBX)
