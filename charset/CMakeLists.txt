include(FindPerl)
if(NOT PERL_EXECUTABLE)
  message(FATAL_ERROR "Perl is required to autogenerate sbcsdat.cpp")
endif()

set(GENERATED_SBCSDAT_C ${GENERATED_SOURCES_DIR}/sbcsdat.cpp)
add_custom_command(OUTPUT ${GENERATED_SBCSDAT_C}.tmp
  COMMAND ${PERL_EXECUTABLE} ${CMAKE_SOURCE_DIR}/charset/sbcsgen.pl
    -o ${GENERATED_SBCSDAT_C}.tmp
  DEPENDS ${CMAKE_SOURCE_DIR}/charset/sbcsgen.pl
          ${CMAKE_SOURCE_DIR}/charset/sbcs.dat)
add_custom_target(generated_sbcsdat_c
  BYPRODUCTS ${GENERATED_SBCSDAT_C}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${GENERATED_SBCSDAT_C}.tmp ${GENERATED_SBCSDAT_C}
  DEPENDS ${GENERATED_SBCSDAT_C}.tmp
  COMMENT "Updating sbcsdat.cpp")

add_library(charset STATIC
  fromucs.cpp
  localenc.cpp
  macenc.cpp
  mimeenc.cpp
  sbcs.cpp
  ${GENERATED_SBCSDAT_C}
  slookup.cpp
  toucs.cpp
  utf8.cpp
  xenc.cpp)
add_dependencies(charset generated_sbcsdat_c)
