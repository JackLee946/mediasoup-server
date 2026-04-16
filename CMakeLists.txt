cmake_minimum_required(VERSION 3.5)

# Detect CPU architecture (cross-platform, no dpkg required)
IF(WIN32)
  # For VS multi-config generators, CMAKE_SIZEOF_VOID_P may not be reliable at configure time.
  # Use CMAKE_GENERATOR_PLATFORM (set by -A x64) or MSVC's target architecture env.
  if(CMAKE_GENERATOR_PLATFORM STREQUAL "x64" OR CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
    if(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
      set(CPU_TYPE "arm")
    else()
      set(CPU_TYPE "x64")
    endif()
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPU_TYPE "x64")
  else()
    set(CPU_TYPE "x86")
  endif()
  message(STATUS "Windows build CPU_TYPE=${CPU_TYPE} (CMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM})")
ELSE()
  # Store in CMAKE_DEB_HOST_ARCH var the current build architecture
  execute_process(COMMAND
    dpkg-architecture
      -qDEB_HOST_ARCH
    OUTPUT_VARIABLE
      CMAKE_DEB_HOST_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(${CMAKE_DEB_HOST_ARCH} MATCHES "arm64")
    set(CPU_TYPE "arm")
    message(STATUS "CMAKE_DEB_HOST_ARCH:${CMAKE_DEB_HOST_ARCH} CPU_TYPE=${CPU_TYPE}")
  elseif(${CMAKE_DEB_HOST_ARCH} MATCHES "i386")
    set(CPU_TYPE "x86")
    message(STATUS "CMAKE_DEB_HOST_ARCH:${CMAKE_DEB_HOST_ARCH} CPU_TYPE=${CPU_TYPE}")
  elseif(${CMAKE_DEB_HOST_ARCH} MATCHES "amd64")
    set(CPU_TYPE "x64")
    message(STATUS "CMAKE_DEB_HOST_ARCH:${CMAKE_DEB_HOST_ARCH} CPU_TYPE=${CPU_TYPE}")
  else()
    set(CPU_TYPE "x64")
    message(STATUS "CMAKE_DEB_HOST_ARCH:${CMAKE_DEB_HOST_ARCH} CPU_TYPE=${CPU_TYPE}")
  endif()
ENDIF()

IF (WIN32)
	#SET(CMAKE_C_COMPILER "c:/Program Files/LLVM/bin/clang")
	#SET(CMAKE_CXX_COMPILER "c:/Program Files/LLVM/bin/clang++")
ELSEIF (APPLE)

ELSEIF (UNIX)
  #find_package(LLVM REQUIRED CONFIG)
  message(STATUS "This is BINARY dir " ${PROJECT_BINARY_DIR})
  message(STATUS "This is SOURCE dir " ${PROJECT_SOURCE_DIR})
  #message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  #message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
  #include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/llvm.toolchain.cmake)

ENDIF ()
message(STATUS "CMAKE_C_COMPILER:${CMAKE_C_COMPILER}") 
message(STATUS "CMAKE_CXX_COMPILER:${CMAKE_CXX_COMPILER}") 
set(PROJECT_NAME "mediasoup-server")
project(${PROJECT_NAME})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/release/include 
${CMAKE_CURRENT_SOURCE_DIR}/release/include/oatpp-1.4.0/oatpp
${CMAKE_CURRENT_SOURCE_DIR}/release/include/oatpp-1.4.0/oatpp-openssl
${CMAKE_CURRENT_SOURCE_DIR}/release/include/oatpp-1.4.0/oatpp-websocket
${CMAKE_CURRENT_SOURCE_DIR}/worker/include 
${CMAKE_CURRENT_SOURCE_DIR}/release/include/concurrentqueue
${CMAKE_CURRENT_SOURCE_DIR}/deps/json/single_include
)
set(PROJECT_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/release/lib)
set(WORKER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/worker/include)
set(LIB_WEBRTC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/libwebrtc ${CMAKE_CURRENT_SOURCE_DIR}/deps/libwebrtc/libwebrtc ${CMAKE_CURRENT_SOURCE_DIR}/deps/libwebrtc/deps/abseil-cpp)
set(PROJECT_RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/release)
set(CONTROLLER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/controller)

IF (WIN32)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/boost/include)
  # OpenSSL: main include + generated config for VC-WIN64A (no-asm)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/openssl/include)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/openssl/generated-config/archs/VC-WIN64A/no-asm/include)
  # libsrtp: root contains srtp.h, include/ subdir also needed
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/libsrtp)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/libsrtp/include)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/libuv/include)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/oatpp/src)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/oatpp-openssl/src)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/oatpp-websocket/src)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/libsdptransform)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/netstring)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/usrsctp/usrsctplib)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/json/single_include)
  # flatbuffers headers
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/flatbuffers/include)
  # sigslot headers
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/sigslot/include)
  # abseil-cpp (standalone, not the one inside libwebrtc)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/abseil-cpp)
  # moodycamel concurrentqueue (included as moodycamel/concurrentqueue.h)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps)
  set(PROJECT_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/${CPU_TYPE})
ENDIF ()

IF(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /Zc:__cplusplus")
    # Suppress common MSVC warnings for third-party code
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
ELSEIF(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
ENDIF()
message("\n=========== mediasoup-server Build Configuration ===========\n")
message(STATUS "PROJECT_INCLUDE_DIR : " ${PROJECT_INCLUDE_DIR})
message(STATUS "PROJECT_LIB_DIR : " ${PROJECT_LIB_DIR})

message("")


# add subdirectory
IF(WIN32)
  # Build all dependencies from source on Windows
  add_subdirectory(deps/libuv)

  # getopt (POSIX getopt for Windows)
  add_subdirectory(deps/getopt)

  # libsrtp2 (needs OpenSSL)
  set(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/openssl CACHE PATH "" FORCE)
  set(OPENSSL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/openssl/include
      ${CMAKE_CURRENT_SOURCE_DIR}/deps/openssl/generated-config/archs/VC-WIN64A/no-asm/include
      CACHE PATH "" FORCE)
  set(TEST_APPS OFF CACHE INTERNAL "")
  set(ENABLE_OPENSSL OFF CACHE INTERNAL "")
  add_subdirectory(deps/libsrtp)
  unset(TEST_APPS)
  unset(ENABLE_OPENSSL)

  # usrsctp
  set(sctp_werror OFF CACHE INTERNAL "")
  set(sctp_build_programs OFF CACHE INTERNAL "")
  set(sctp_build_fuzzer OFF CACHE INTERNAL "")
  set(sctp_link_programs_static OFF CACHE INTERNAL "")
  add_subdirectory(deps/usrsctp)
  unset(sctp_werror)
  unset(sctp_build_programs)
  unset(sctp_build_fuzzer)
  unset(sctp_link_programs_static)

  # oatpp
  set(OATPP_BUILD_TESTS OFF CACHE INTERNAL "")
  set(OATPP_INSTALL OFF CACHE INTERNAL "")
  add_subdirectory(deps/oatpp)
  unset(OATPP_BUILD_TESTS)
  unset(OATPP_INSTALL)

  # oatpp-websocket (CUSTOM mode: point to already-added oatpp sources)
  set(OATPP_MODULES_LOCATION "CUSTOM" CACHE STRING "" FORCE)
  set(OATPP_DIR_SRC "${CMAKE_CURRENT_SOURCE_DIR}/deps/oatpp/src" CACHE PATH "" FORCE)
  set(OATPP_BUILD_TESTS OFF CACHE INTERNAL "")
  add_subdirectory(deps/oatpp-websocket)
  # oatpp-openssl requires pre-built OpenSSL; skip for Windows debug builds
  # add_subdirectory(deps/oatpp-openssl)
  unset(OATPP_MODULES_LOCATION)
  unset(OATPP_DIR_SRC)

  # abseil-cpp
  set(ABSL_BUILD_TESTING OFF CACHE INTERNAL "")
  set(ABSL_ENABLE_INSTALL OFF CACHE INTERNAL "")
  add_subdirectory(deps/abseil-cpp)
  unset(ABSL_BUILD_TESTING)
  unset(ABSL_ENABLE_INSTALL)

ELSE()
  # Non-Windows: dependencies are pre-built
ENDIF()

add_subdirectory(deps/libwebrtc)
add_subdirectory(worker)
add_subdirectory(controller)
add_subdirectory(sfu)

add_dependencies(sfu controller worker webrtc)
