set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_sources_from_current_dir(utils
  utils/arm_arch_queries.cpp
  utils/block_signal.cpp
  utils/cloexec.cpp
  utils/cmdline_arg.cpp
  utils/dputs.cpp
  utils/filename.cpp
  utils/fontspec.cpp
  utils/getticks.cpp
  utils/get_username.cpp
  utils/keysym_to_unicode.cpp
  utils/make_dir_and_check_ours.cpp
  utils/make_dir_path.cpp
  utils/make_spr_sw_abort_errno.cpp
  utils/nonblock.cpp
  utils/open_for_write_would_lose_data.cpp
  utils/pgp_fingerprints.cpp
  utils/pollwrap.cpp
  utils/signal.cpp
  utils/subprocess_waiter.cpp
  utils/x11_ignore_error.cpp
  # We want the ISO C implementation of ltime(), because we don't have
  # a local better alternative
  ../utils/ltime.cpp)
# Compiled icon pixmap files
add_library(puttyxpms STATIC
  putty-xpm.cpp
  putty-rgb.cpp
  putty-config-xpm.cpp
  putty-config-rgb.cpp)
add_library(ptermxpms STATIC
  pterm-xpm.cpp
  pterm-rgb.cpp
  pterm-config-xpm.cpp
  pterm-config-rgb.cpp)
add_sources_from_current_dir(eventloop
  cliloop.cpp uxsel.cpp)
add_sources_from_current_dir(console
  console.cpp)
add_sources_from_current_dir(settings
  storage.cpp)
add_sources_from_current_dir(network
  network.cpp fd-socket.cpp agent-socket.cpp peerinfo.cpp local-proxy.cpp x11.cpp)
add_sources_from_current_dir(sshcommon
  noise.cpp)
add_sources_from_current_dir(sshclient
  gss.cpp agent-client.cpp sharing.cpp)
add_sources_from_current_dir(sshserver
  sftpserver.cpp procnet.cpp)
add_sources_from_current_dir(sftpclient
  sftp.cpp)
add_sources_from_current_dir(otherbackends
  serial.cpp)
add_sources_from_current_dir(agent
  agent-client.cpp)

add_executable(fuzzterm
  ${CMAKE_SOURCE_DIR}/test/fuzzterm.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-print.cpp
  unicode.cpp
  no-gtk.cpp
  $<TARGET_OBJECTS:logging>)
be_list(fuzzterm FuZZterm)
add_dependencies(fuzzterm generated_licence_h)
target_link_libraries(fuzzterm
  guiterminal eventloop charset settings utils)

add_executable(osxlaunch
  osxlaunch.cpp)

add_sources_from_current_dir(plink unicode.cpp no-gtk.cpp)
add_sources_from_current_dir(pscp unicode.cpp no-gtk.cpp)
add_sources_from_current_dir(psftp unicode.cpp no-gtk.cpp)
add_sources_from_current_dir(psocks no-gtk.cpp)

add_executable(psusan
  psusan.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
  ${CMAKE_SOURCE_DIR}/ssh/scpserver.cpp
  unicode.cpp
  no-gtk.cpp
  pty.cpp)
be_list(psusan psusan)
target_link_libraries(psusan
  eventloop sshserver keygen settings network crypto utils)
installed_program(psusan)

add_library(puttygen-common OBJECT
  ${CMAKE_SOURCE_DIR}/stubs/no-timing.cpp
  keygen-noise.cpp
  no-gtk.cpp
  noise.cpp
  storage.cpp
  ${CMAKE_SOURCE_DIR}/sshpubk.cpp
  ${CMAKE_SOURCE_DIR}/sshrand.cpp)

add_executable(puttygen
  ${CMAKE_SOURCE_DIR}/cmdgen.cpp
  $<TARGET_OBJECTS:puttygen-common>)
target_link_libraries(puttygen keygen console crypto utils)
installed_program(puttygen)

add_executable(cgtest
  ${CMAKE_SOURCE_DIR}/cgtest.cpp
  $<TARGET_OBJECTS:puttygen-common>)
target_link_libraries(cgtest keygen console crypto utils)

if(HAVE_ALIGNED_ALLOC)
  add_library(overaligned_alloc OBJECT
    ${CMAKE_SOURCE_DIR}/utils/memory.cpp)
  target_compile_definitions(overaligned_alloc PRIVATE ALLOCATION_ALIGNMENT=128)
  add_executable(testsc
    ${CMAKE_SOURCE_DIR}/test/testsc.cpp
    ${CMAKE_SOURCE_DIR}/test/test_random.cpp
    $<TARGET_OBJECTS:overaligned_alloc>)
  target_link_libraries(testsc keygen crypto utils)
endif()

add_executable(testzlib
  ${CMAKE_SOURCE_DIR}/test/testzlib.cpp
  ${CMAKE_SOURCE_DIR}/ssh/zlib.cpp)
target_link_libraries(testzlib utils)

add_executable(uppity
  uppity.cpp
  ${CMAKE_SOURCE_DIR}/ssh/scpserver.cpp
  no-gtk.cpp
  pty.cpp
  unicode.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp)
be_list(uppity Uppity)
target_link_libraries(uppity
  eventloop sshserver keygen settings network crypto utils)

if(GTK_FOUND)
  add_sources_from_current_dir(utils
    utils/align_label_left.cpp
    utils/buildinfo_gtk_version.cpp
    utils/get_label_text_dimensions.cpp
    utils/get_x11_display.cpp
    utils/our_dialog.cpp
    utils/string_width.cpp
    columns.cpp)
  add_sources_from_current_dir(guiterminal
    window.cpp unifont.cpp dialog.cpp config-gtk.cpp gtk-common.cpp config-unix.cpp unicode.cpp printing.cpp)
  add_dependencies(guiterminal generated_licence_h) # dialog.cpp uses licence.h

  add_executable(pterm
    pterm.cpp
    main-gtk-simple.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-ca-config.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-console.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-dit.cpp
    ${CMAKE_SOURCE_DIR}/proxy/nosshproxy.cpp
    pty.cpp)
  be_list(pterm pterm)
  target_link_libraries(pterm
    guiterminal eventloop settings utils ptermxpms
    ${GTK_LIBRARIES} ${X11_LIBRARIES} ${X11_Xrender_LIB})
  installed_program(pterm)

  if(GTK_VERSION GREATER_EQUAL 3)
    add_executable(ptermapp
      pterm.cpp
      main-gtk-application.cpp
      ${CMAKE_SOURCE_DIR}/stubs/no-cmdline.cpp
      ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
      ${CMAKE_SOURCE_DIR}/stubs/no-ca-config.cpp
      ${CMAKE_SOURCE_DIR}/stubs/no-console.cpp
      ${CMAKE_SOURCE_DIR}/stubs/no-dit.cpp
      ${CMAKE_SOURCE_DIR}/proxy/nosshproxy.cpp
      pty.cpp)
    be_list(ptermapp pterm)
    target_link_libraries(ptermapp
      guiterminal eventloop settings utils ptermxpms
      ${GTK_LIBRARIES} ${X11_LIBRARIES} ${X11_Xrender_LIB})
  endif()

  add_executable(putty
    putty.cpp
    main-gtk-simple.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-console.cpp)
  be_list(putty PuTTY SSH SERIAL OTHERBACKENDS)
  target_link_libraries(putty
    guiterminal eventloop sshclient otherbackends settings
    network crypto utils puttyxpms
    ${GTK_LIBRARIES} ${X11_LIBRARIES} ${X11_Xrender_LIB})
  set_target_properties(putty
    PROPERTIES LINK_INTERFACE_MULTIPLICITY 2)
  installed_program(putty)

  if(GTK_VERSION GREATER_EQUAL 3)
    add_executable(puttyapp
      putty.cpp
      main-gtk-application.cpp
      ${CMAKE_SOURCE_DIR}/stubs/no-cmdline.cpp
      ${CMAKE_SOURCE_DIR}/stubs/no-console.cpp)
    be_list(puttyapp PuTTY SSH SERIAL OTHERBACKENDS)
    target_link_libraries(puttyapp
      guiterminal eventloop sshclient otherbackends settings
      network crypto utils puttyxpms
      ${GTK_LIBRARIES} ${X11_LIBRARIES} ${X11_Xrender_LIB})
  endif()

  add_executable(puttytel
    putty.cpp
    main-gtk-simple.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-ca-config.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-console.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-rand.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-dit.cpp
    ${CMAKE_SOURCE_DIR}/proxy/nocproxy.cpp
    ${CMAKE_SOURCE_DIR}/proxy/nosshproxy.cpp)
  be_list(puttytel PuTTYtel SERIAL OTHERBACKENDS)
  target_link_libraries(puttytel
    guiterminal eventloop otherbackends settings network utils
    puttyxpms
    ${GTK_LIBRARIES} ${X11_LIBRARIES} ${X11_Xrender_LIB})

  add_executable(test_lineedit
    ${CMAKE_SOURCE_DIR}/test/test_lineedit.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-logging.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-printing.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-storage.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-timing.cpp)
  target_link_libraries(test_lineedit
    guiterminal settings eventloop utils ${platform_libraries})

  add_executable(test_terminal
    ${CMAKE_SOURCE_DIR}/test/test_terminal.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-storage.cpp
    ${CMAKE_SOURCE_DIR}/stubs/no-timing.cpp)
  target_link_libraries(test_terminal
    guiterminal settings eventloop utils ${platform_libraries})
endif()

# Pageant is built whether we have GTK or not; in its absence we
# degrade to a version that doesn't provide the GTK askpass.
if(GTK_FOUND)
  set(pageant_conditional_sources askpass.cpp)
  set(pageant_libs ${GTK_LIBRARIES})
else()
  set(pageant_conditional_sources noaskpass.cpp no-gtk.cpp)
  set(pageant_libs)
endif()
add_executable(pageant
  pageant.cpp
  ${CMAKE_SOURCE_DIR}/stubs/no-gss.cpp
  x11.cpp
  noise.cpp
  ${CMAKE_SOURCE_DIR}/ssh/x11fwd.cpp
  ${CMAKE_SOURCE_DIR}/proxy/nosshproxy.cpp
  ${pageant_conditional_sources})
be_list(pageant Pageant)
target_link_libraries(pageant
  eventloop console agent settings network crypto utils
  ${pageant_libs})
installed_program(pageant)

add_sources_from_current_dir(test_conf unicode.cpp stubs/no-uxsel.cpp)
