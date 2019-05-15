project (libde265)
cmake_minimum_required (VERSION 2.8.8)

# The version number.
set (NUMERIC_VERSION 0x01000000)
set (PACKAGE_VERSION 1.0.0)

include (${CMAKE_ROOT}/Modules/CheckCCompilerFlag.cmake)
include (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
include (${CMAKE_ROOT}/Modules/FindSDL.cmake)
include (${CMAKE_ROOT}/Modules/FindThreads.cmake)

CHECK_INCLUDE_FILE(malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stdbool.h HAVE_STDBOOL_H)

if (HAVE_MALLOC_H)
  add_definitions(-DHAVE_MALLOC_H)
endif()
if (HAVE_STDINT_H)
  add_definitions(-DHAVE_STDINT_H)
endif()
if (HAVE_STDBOOL_H)
  add_definitions(-DHAVE_STDBOOL_H)
endif()

configure_file (
  "${PROJECT_SOURCE_DIR}/libde265/de265-version.h.in"
  "${PROJECT_BINARY_DIR}/libde265/de265-version.h"
)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(GCC 1)
  add_definitions(-Wall)
  set(CMAKE_CXX_FLAGS "-std=gnu++0x ${CMAKE_CXX_FLAGS}")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  add_definitions(-Wall)
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

option(DISABLE_SSE "Disable SSE optimizations")
if(NOT ${DISABLE_SSE} EQUAL OFF)
  if(MSVC)
    set(SUPPORTS_SSE4_1 1)
  else()
    CHECK_C_COMPILER_FLAG(-msse4.1 SUPPORTS_SSE4_1)
  endif()
endif()

include_directories ("${PROJECT_SOURCE_DIR}")
include_directories ("${PROJECT_BINARY_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}/libde265")
if(MSVC)
  include_directories ("${PROJECT_SOURCE_DIR}/extra")
  add_definitions(-DHAVE_STDINT_H)
  add_definitions(-DHAVE_STDBOOL_H)
  add_definitions(-DNOMINMAX)
endif()

if(UNIX)
  set(LIBDE265_LIBRARY_NAME de265)
else()
  set(LIBDE265_LIBRARY_NAME libde265)
endif()

add_subdirectory (libde265)
add_subdirectory (dec265)
add_subdirectory (enc265)
