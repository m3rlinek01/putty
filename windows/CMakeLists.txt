set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_sources_from_current_dir(utils
  utils/agent_mutex_name.cpp
  utils/agent_named_pipe_name.cpp
  utils/arm_arch_queries.cpp
  utils/aux_match_opt.cpp
  utils/blink_time.cpp
  utils/centre_window.cpp
  utils/cmdline_arg.cpp
  utils/cryptoapi.cpp
  utils/defaults.cpp
  utils/dll_hijacking_protection.cpp
  utils/dputs.cpp
  utils/escape_registry_key.cpp
  utils/filename.cpp
  utils/fontspec.cpp
  utils/getdlgitemtext_alloc.cpp
  utils/get_system_dir.cpp
  utils/get_username.cpp
  utils/gui-timing.cpp
  utils/interprocess_mutex.cpp
  utils/is_console_handle.cpp
  utils/load_system32_dll.cpp
  utils/ltime.cpp
  utils/makedlgitemborderless.cpp
  utils/make_spr_sw_abort_winerror.cpp
  utils/message_box.cpp
  utils/minefield.cpp
  utils/open_for_write_would_lose_data.cpp
  utils/pgp_fingerprints_msgbox.cpp
  utils/platform_get_x_display.cpp
  utils/registry.cpp
  utils/request_file.cpp
  utils/screenshot.cpp
  utils/security.cpp
  utils/shinydialogbox.cpp
  utils/split_into_argv.cpp
  utils/split_into_argv_w.cpp
  utils/subprocess_waiter.cpp
  utils/version.cpp
  utils/win_strerror.cpp
  unicode.cpp)
if(NOT HAVE_STRTOUMAX)
  add_sources_from_current_dir(utils utils/strtoumax.cpp)
endif()
add_sources_from_current_dir(eventloop
  cliloop.cpp handle-wait.cpp)
add_sources_from_current_dir(console
  select-cli.cpp nohelp.cpp console.cpp)
add_sources_from_current_dir(settings
  storage.cpp)
add_sources_from_current_dir(network
  network.cpp handle-socket.cpp named-pipe-client.cpp named-pipe-server.cpp local-proxy.cpp x11.cpp)
add_sources_from_current_dir(sshcommon
  noise.cpp)
add_sources_from_current_dir(sshclient
  agent-client.cpp gss.cpp sharing.cpp)
add_sources_from_current_dir(sftpclient
  sftp.cpp)
add_sources_from_current_dir(otherbackends
  serial.cpp)
add_sources_from_current_dir(agent
  agent-client.cpp)
add_sources_from_current_dir(guiterminal
  dialog.cpp controls.cpp config.cpp printing.cpp jump-list.cpp sizetip.cpp)
add_dependencies(guiterminal generated_licence_h) # dialog.cpp uses licence.h

# This object awkwardly needs to live in the network library as well
# as the eventloop library, in case it didn't get pulled in from the
# latter before handle-socket.cpp needed it.
add_library(handle-io OBJECT
  handle-io.cpp)
target_sources(eventloop PRIVATE $<TARGET_OBJECTS:handle-io>)
target_sources(network PRIVATE $<TARGET_OBJECTS:handle-io>)

add_library(guimisc STATIC
  select-gui.cpp)

add_executable(pageant
  pageant.cpp
  help.cpp
  pageant.rc)
add_dependencies(pageant generated_licence_h)
target_link_libraries(pageant
  guimisc eventloop agent network crypto utils
  ${platform_libraries})
set_target_properties(pageant PROPERTIES WIN32_EXECUTABLE ON)
installed_program(pageant)

add_sources_from_current_dir(plink no-jump-list.cpp nohelp.cpp plink.rc)
add_dependencies(plink generated_licence_h)

add_sources_from_current_dir(pscp no-jump-list.cpp nohelp.cpp pscp.rc)
add_dependencies(pscp generated_licence_h)

add_sources_from_current_dir(psftp no-jump-list.cpp nohelp.cpp psftp.rc)
add_dependencies(psftp generated_licence_h)

add_sources_from_current_dir(psocks nohelp.cpp)

add_executable(putty
  window.cpp
  putty.cpp
  help.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-console.cpp
  putty.rc)
be_list(putty PuTTY SSH SERIAL OTHERBACKENDS)
add_dependencies(putty generated_licence_h)
target_link_libraries(putty
  guiterminal guimisc eventloop sshclient otherbackends settings network crypto
  utils
  ${platform_libraries})
set_target_properties(putty PROPERTIES WIN32_EXECUTABLE ON)
installed_program(putty)

add_executable(puttytel
  window.cpp
  putty.cpp
  help.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-ca-config.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-console.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-rand.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-dit.cpp
  ${CMAKE_SOURCE_DIR}/proxy/nocproxy.cpp
  ${CMAKE_SOURCE_DIR}/proxy/nosshproxy.cpp
  puttytel.rc)
be_list(puttytel PuTTYtel SERIAL OTHERBACKENDS)
add_dependencies(puttytel generated_licence_h)
target_link_libraries(puttytel
  guiterminal guimisc eventloop otherbackends settings network utils
  ${platform_libraries})
set_target_properties(puttytel PROPERTIES WIN32_EXECUTABLE ON)
installed_program(puttytel)

add_executable(puttygen
  puttygen.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-timing.cpp
  noise.cpp
  no-jump-list.cpp
  storage.cpp
  help.cpp
  ${CMAKE_SOURCE_DIR}/sshpubk.cpp
  ${CMAKE_SOURCE_DIR}/sshrand.cpp
  controls.cpp
  puttygen.rc)
add_dependencies(puttygen generated_licence_h)
target_link_libraries(puttygen
  keygen guimisc crypto utils
  ${platform_libraries})
set_target_properties(puttygen PROPERTIES WIN32_EXECUTABLE ON)
installed_program(puttygen)

if(HAVE_CONPTY)
  add_executable(pterm
    window.cpp
    pterm.cpp
    help.cpp
    conpty.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-ca-config.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-console.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-rand.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-dit.cpp
    ${CMAKE_SOURCE_DIR}/proxy/nosshproxy.cpp
    pterm.rc)
  be_list(pterm pterm)
  add_dependencies(pterm generated_licence_h)
  target_link_libraries(pterm
    guiterminal guimisc eventloop settings network utils
    ${platform_libraries})
  set_target_properties(pterm PROPERTIES WIN32_EXECUTABLE ON)
  installed_program(pterm)
else()
  message("ConPTY not available; cannot build Windows pterm")
endif()

add_executable(test_split_into_argv
  test/test_split_into_argv.cpp)
target_compile_definitions(test_split_into_argv PRIVATE TEST)
target_link_libraries(test_split_into_argv utils ${platform_libraries})

add_executable(test_screenshot
  test/test_screenshot.cpp)
target_link_libraries(test_screenshot utils ${platform_libraries})

add_executable(test_lineedit
  ${CMAKE_SOURCE_DIR}/test/test_lineedit.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-logging.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-printing.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-storage.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-timing.cpp
  no-jump-list.cpp)
target_link_libraries(test_lineedit
  guiterminal settings eventloop utils ${platform_libraries})

add_executable(test_terminal
  ${CMAKE_SOURCE_DIR}/test/test_terminal.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-storage.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-timing.cpp
  no-jump-list.cpp)
target_link_libraries(test_terminal
  guiterminal settings eventloop utils ${platform_libraries})

add_sources_from_current_dir(test_conf no-jump-list.cpp handle-wait.cpp)
