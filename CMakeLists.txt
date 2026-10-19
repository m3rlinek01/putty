cmake_minimum_required(VERSION 3.21...3.31)
project(putty LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")

if(WIN32 AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(FATAL_ERROR "The Windows C++ port requires LLVM-MinGW Clang. Use build-cpp.ps1.")
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # Ship standalone EXEs: STL use must not add toolchain DLL dependencies.
  # Windows system/UCRT imports remain dynamic as before.
  add_link_options(-static)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-strict-aliasing>)
endif()

set(CMAKE_C_STANDARD 99)

include(cmake/setup.cmake)

# Scan the docs directory first, so that when we start calling
# installed_program(), we'll know if we have man pages available
add_subdirectory(doc)

add_compile_definitions(HAVE_CMAKE_H)

include_directories(terminal)

add_library(utils STATIC
  ${GENERATED_COMMIT_C})
add_dependencies(utils cmake_commit_c)
add_subdirectory(utils)
add_subdirectory(stubs)

add_library(logging OBJECT
  logging.cpp utils/logeventf.cpp)

add_library(eventloop STATIC
  callback.cpp timing.cpp)

add_library(console STATIC
  clicons.cpp console.cpp)

add_library(settings STATIC
  cmdline.cpp settings.cpp)

add_library(crypto STATIC
  proxy/cproxy.cpp proxy/sshproxy.cpp)
add_subdirectory(crypto)

add_library(network STATIC
  errsock.cpp x11disp.cpp
  $<TARGET_OBJECTS:logging>
  proxy/proxy.cpp
  proxy/http.cpp
  proxy/socks4.cpp
  proxy/socks5.cpp
  proxy/telnet.cpp
  proxy/newmain.cpp
  proxy/local.cpp
  proxy/interactor.cpp)

add_library(keygen STATIC
  import.cpp)
add_subdirectory(keygen)

add_library(agent STATIC
  sshpubk.cpp pageant.cpp aqsync.cpp)

add_library(guiterminal STATIC
  terminal/terminal.cpp terminal/bidi.cpp
  ldisc.cpp terminal/lineedit.cpp config.cpp dialog.cpp
  $<TARGET_OBJECTS:logging>)

add_library(noterminal STATIC
  stubs/no-term.cpp ldisc.cpp)

add_library(all-backends OBJECT
  pinger.cpp)

add_library(sftpclient STATIC
  psftpcommon.cpp)
add_subdirectory(ssh)

add_library(otherbackends STATIC
  $<TARGET_OBJECTS:all-backends>
  $<TARGET_OBJECTS:logging>)
add_subdirectory(otherbackends)

add_executable(testcrypt
  test/testcrypt.cpp sshpubk.cpp ssh/crc-attack-detector.cpp)
target_link_libraries(testcrypt
  keygen crypto utils ${platform_libraries})

add_executable(test_bpp
  test/test_bpp.cpp
  test/test_random.cpp
  callback.cpp
  stubs/no-logging.cpp
  stubs/no-network.cpp
  stubs/no-agent.cpp
  stubs/no-ldisc.cpp
  stubs/no-timing.cpp
  stubs/no-storage.cpp
  stubs/no-ca-config.cpp
  proxy/noproxy.cpp)
target_link_libraries(test_bpp
  sshclient crypto network utils ${platform_libraries})

add_executable(test_host_strfoo
  utils/host_strchr_internal.cpp)
target_compile_definitions(test_host_strfoo PRIVATE TEST)
target_link_libraries(test_host_strfoo utils ${platform_libraries})

add_executable(test_decode_utf8
  utils/decode_utf8.cpp)
target_compile_definitions(test_decode_utf8 PRIVATE TEST)
target_link_libraries(test_decode_utf8 utils ${platform_libraries})

add_executable(test_unicode_norm
  utils/unicode-norm.cpp)
target_compile_definitions(test_unicode_norm PRIVATE TEST)
target_link_libraries(test_unicode_norm utils ${platform_libraries})

add_executable(test_tree234
  utils/tree234.cpp)
target_compile_definitions(test_tree234 PRIVATE TEST)
target_link_libraries(test_tree234 utils ${platform_libraries})

add_executable(test_wildcard
  utils/wildcard.cpp)
target_compile_definitions(test_wildcard PRIVATE TEST)
target_link_libraries(test_wildcard utils ${platform_libraries})

add_executable(test_cert_expr
  utils/cert-expr.cpp)
target_compile_definitions(test_cert_expr PRIVATE TEST)
target_link_libraries(test_cert_expr utils ${platform_libraries})

add_executable(bidi_gettype
  terminal/bidi_gettype.cpp)
target_link_libraries(bidi_gettype guiterminal utils ${platform_libraries})

add_executable(bidi_test
  terminal/bidi_test.cpp)
target_link_libraries(bidi_test guiterminal utils ${platform_libraries})

add_executable(plink
  ${platform}/plink.cpp
  stubs/no-lineedit.cpp)
# Note: if we ever port Plink to a platform where we can't implement a
# serial backend, this be_list command will need to become platform-
# dependent, so that it only sets the SERIAL option on platforms where
# that backend exists. For the moment, though, we have serial port
# backends for both our platforms, so we can do this unconditionally.
be_list(plink Plink SSH SERIAL OTHERBACKENDS)
target_link_libraries(plink
  eventloop noterminal console sshclient otherbackends settings network crypto
  utils
  ${platform_libraries})
installed_program(plink)

add_executable(pscp
  pscp.cpp)
be_list(pscp PSCP SSH)
target_link_libraries(pscp
  sftpclient eventloop console sshclient settings network crypto utils
  ${platform_libraries})
installed_program(pscp)

add_executable(psftp
  psftp.cpp)
be_list(psftp PSFTP SSH)
target_link_libraries(psftp
  sftpclient eventloop console sshclient settings network crypto utils
  ${platform_libraries})
installed_program(psftp)

add_executable(psocks
  ${platform}/psocks.cpp
  psocks.cpp
  stubs/no-rand.cpp
  proxy/nocproxy.cpp
  proxy/nosshproxy.cpp
  ssh/portfwd.cpp)
target_link_libraries(psocks
  eventloop console network utils
  ${platform_libraries})

add_executable(test_conf
  test/test_conf.cpp
  stubs/no-agent.cpp
  stubs/no-callback.cpp
  stubs/no-gss.cpp
  stubs/no-ldisc.cpp
  stubs/no-network.cpp
  stubs/no-timing.cpp
  proxy/noproxy.cpp # FIXME: move this to stubs
)
be_list(test_conf TestConf SSH SERIAL OTHERBACKENDS)
target_link_libraries(test_conf sshclient otherbackends settings network crypto utils ${platform_libraries})

foreach(subdir ${platform} ${extra_dirs})
  add_subdirectory(${subdir})
endforeach()

# Nasty bodge: we'd like to run this command inside unix/CMakeLists,
# adding the 'charset' library to everything that links with utils.
# But that wasn't allowed until cmake 3.13 (see cmake policy CMP0073),
# and we still have a min cmake version less than that. So we do it
# here instead.
if(platform STREQUAL unix)
  target_link_libraries(utils charset)
endif()

configure_file(cmake/cmake.h.in ${GENERATED_SOURCES_DIR}/cmake.h)

include(cmake/port-tests.cmake)
