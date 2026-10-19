enable_testing()

foreach(name host_strfoo decode_utf8 tree234 wildcard cert_expr conf bpp
             split_into_argv lineedit terminal)
  add_test(NAME ${name} COMMAND $<TARGET_FILE:test_${name}>)
  set_tests_properties(${name} PROPERTIES TIMEOUT 60)
endforeach()

if(TARGET test_file_transfer)
  add_test(NAME file_transfer COMMAND $<TARGET_FILE:test_file_transfer>)
  set_tests_properties(file_transfer PROPERTIES TIMEOUT 60)
endif()

find_package(Python3 QUIET COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  add_test(NAME crypto
    COMMAND ${CMAKE_COMMAND} -E env "PUTTY_TESTCRYPT_EXE=$<TARGET_FILE:testcrypt>"
    ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/cryptsuite.py -v)
  set_tests_properties(crypto PROPERTIES TIMEOUT 180)

  execute_process(COMMAND ${Python3_EXECUTABLE} -c "import paramiko"
    RESULT_VARIABLE has_no_paramiko OUTPUT_QUIET ERROR_QUIET)
  if(has_no_paramiko EQUAL 0)
    add_test(NAME local_ssh_sftp
      COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/port_integration.py
      $<TARGET_FILE_DIR:plink>)
    set_tests_properties(local_ssh_sftp PROPERTIES TIMEOUT 180)
  else()
    message(STATUS "Optional SSH/SFTP integration test: install Paramiko in the selected Python environment")
  endif()
endif()
