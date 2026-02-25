cmake_minimum_required(VERSION 3.24)

project(IvyCpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(IVY_BUILD_TESTS "Build C++ tests" ON)
option(IVY_BUILD_CLI "Build CLI executable" ON)
option(IVY_BUILD_SAMPLE_APP "Build sample app" ON)
option(IVY_ENABLE_WARNINGS "Enable strict warnings" ON)
option(IVY_ENABLE_ASAN_UBSAN "Enable AddressSanitizer and UBSan" OFF)
option(IVY_ENABLE_COVERAGE "Enable source coverage flags" OFF)
option(IVY_ENABLE_FUZZ "Build parser fuzz target" OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(FetchContent)

if(IVY_BUILD_TESTS)
  include(CTest)
endif()

find_package(Java REQUIRED COMPONENTS Runtime)

find_package(Z3 CONFIG QUIET)
if(NOT TARGET z3::libz3)
  find_path(IVY_Z3_INCLUDE_DIR NAMES z3++.h)
  find_library(IVY_Z3_LIBRARY NAMES z3 libz3)
  if(NOT IVY_Z3_INCLUDE_DIR OR NOT IVY_Z3_LIBRARY)
    message(FATAL_ERROR "Could not find Z3. Install Z3 development files (headers + library).")
  endif()
  add_library(z3::libz3 UNKNOWN IMPORTED)
  set_target_properties(
    z3::libz3 PROPERTIES
    IMPORTED_LOCATION "${IVY_Z3_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${IVY_Z3_INCLUDE_DIR}")
endif()

set(IVY_ANTLR_JAR "/Users/nibom/v23/antlr-4.13.2-complete.jar" CACHE FILEPATH "Path to antlr-4.13.2-complete.jar")
set(IVY_GRAMMAR "${CMAKE_SOURCE_DIR}/src/main/java/imp/parser/antlr/Imp.g4")
set(IVY_ANTLR_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated/antlr")

if(NOT EXISTS "${IVY_ANTLR_JAR}")
  message(FATAL_ERROR "ANTLR jar not found at ${IVY_ANTLR_JAR}")
endif()

if(NOT EXISTS "${IVY_GRAMMAR}")
  message(FATAL_ERROR "Grammar file not found at ${IVY_GRAMMAR}")
endif()

file(MAKE_DIRECTORY "${IVY_ANTLR_OUTPUT_DIR}")

set(ANTLR_GENERATED_SOURCES
    "${IVY_ANTLR_OUTPUT_DIR}/ImpLexer.cpp"
    "${IVY_ANTLR_OUTPUT_DIR}/ImpParser.cpp"
    "${IVY_ANTLR_OUTPUT_DIR}/ImpBaseVisitor.cpp"
    "${IVY_ANTLR_OUTPUT_DIR}/ImpVisitor.cpp")

set(ANTLR_GENERATED_HEADERS
    "${IVY_ANTLR_OUTPUT_DIR}/ImpLexer.h"
    "${IVY_ANTLR_OUTPUT_DIR}/ImpParser.h"
    "${IVY_ANTLR_OUTPUT_DIR}/ImpBaseVisitor.h"
    "${IVY_ANTLR_OUTPUT_DIR}/ImpVisitor.h")

add_custom_command(
  OUTPUT ${ANTLR_GENERATED_SOURCES} ${ANTLR_GENERATED_HEADERS}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${IVY_GRAMMAR}" "${IVY_ANTLR_OUTPUT_DIR}/Imp.g4"
  COMMAND ${Java_JAVA_EXECUTABLE} -jar "${IVY_ANTLR_JAR}" -Dlanguage=Cpp -visitor -no-listener
          -o "${IVY_ANTLR_OUTPUT_DIR}" "${IVY_ANTLR_OUTPUT_DIR}/Imp.g4"
  DEPENDS "${IVY_GRAMMAR}"
  COMMENT "Generating C++ parser from Imp.g4"
  VERBATIM)

add_custom_target(ivy_antlr_gen DEPENDS ${ANTLR_GENERATED_SOURCES} ${ANTLR_GENERATED_HEADERS})

set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "" FORCE)
set(ANTLR4_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  antlr4_runtime
  GIT_REPOSITORY https://github.com/antlr/antlr4.git
  GIT_TAG 4.13.2
  SOURCE_SUBDIR runtime/Cpp)
FetchContent_MakeAvailable(antlr4_runtime)

function(ivy_apply_warnings target_name)
  if(NOT IVY_ENABLE_WARNINGS)
    return()
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    target_compile_options(${target_name} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${target_name} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
  elseif(MSVC)
    target_compile_options(${target_name} PRIVATE /W4 /WX)
  endif()
endfunction()

function(ivy_apply_sanitizers target_name)
  if(NOT IVY_ENABLE_ASAN_UBSAN)
    return()
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
  endif()
endfunction()

function(ivy_apply_coverage target_name)
  if(NOT IVY_ENABLE_COVERAGE)
    return()
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${target_name} PRIVATE --coverage -O0 -g)
    target_link_options(${target_name} PRIVATE --coverage)
  endif()
endfunction()

add_library(ivy SHARED
  src/ast.cpp
  src/pretty_printer.cpp
  src/parser.cpp
  src/type_checker.cpp
  src/z3_interpreter.cpp
  src/verifier.cpp
  ${ANTLR_GENERATED_SOURCES})

add_dependencies(ivy ivy_antlr_gen)

set_source_files_properties(${ANTLR_GENERATED_SOURCES} PROPERTIES COMPILE_OPTIONS "-w")

add_library(Ivy::ivy ALIAS ivy)
set_property(TARGET ivy PROPERTY NO_SYSTEM_FROM_IMPORTED ON)

target_include_directories(ivy
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    "${IVY_ANTLR_OUTPUT_DIR}")

target_link_libraries(ivy
  PUBLIC
    z3::libz3
  PRIVATE
    antlr4_static)

get_target_property(IVY_Z3_INCLUDE_DIRS z3::libz3 INTERFACE_INCLUDE_DIRECTORIES)
if(IVY_Z3_INCLUDE_DIRS)
  target_include_directories(ivy PUBLIC ${IVY_Z3_INCLUDE_DIRS})
endif()

ivy_apply_warnings(ivy)
ivy_apply_sanitizers(ivy)
ivy_apply_coverage(ivy)

if(IVY_BUILD_CLI)
  add_executable(ivy_verify src/main.cpp)
  target_link_libraries(ivy_verify PRIVATE Ivy::ivy)
  ivy_apply_warnings(ivy_verify)
  ivy_apply_sanitizers(ivy_verify)
  ivy_apply_coverage(ivy_verify)
  install(TARGETS ivy_verify RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(IVY_BUILD_SAMPLE_APP)
  add_executable(ivy_sample_app examples/sample_app.cpp)
  target_link_libraries(ivy_sample_app PRIVATE Ivy::ivy)
  ivy_apply_warnings(ivy_sample_app)
  ivy_apply_sanitizers(ivy_sample_app)
  ivy_apply_coverage(ivy_sample_app)
  install(TARGETS ivy_sample_app RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(IVY_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()

  add_executable(ivy_tests
    tests/test_utils.cpp
    tests/test_parser.cpp
    tests/test_branch_coverage.cpp
    tests/test_typechecker.cpp
    tests/test_verifier.cpp)

  target_compile_definitions(ivy_tests PRIVATE IVY_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  target_link_libraries(ivy_tests PRIVATE Ivy::ivy GTest::gtest_main)

  ivy_apply_warnings(ivy_tests)
  ivy_apply_sanitizers(ivy_tests)
  ivy_apply_coverage(ivy_tests)

  include(GoogleTest)
  gtest_discover_tests(ivy_tests DISCOVERY_TIMEOUT 180)
endif()

if(IVY_ENABLE_FUZZ)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    add_executable(ivy_parser_fuzz fuzz/parser_fuzz.cpp)
    target_link_libraries(ivy_parser_fuzz PRIVATE Ivy::ivy)
    target_compile_options(ivy_parser_fuzz PRIVATE -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)
    target_link_options(ivy_parser_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
  else()
    message(WARNING "IVY_ENABLE_FUZZ is ON but this compiler does not support libFuzzer target options here")
  endif()
endif()

install(
  TARGETS ivy
  EXPORT IvyTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT IvyTargets
  FILE IvyTargets.cmake
  NAMESPACE Ivy::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Ivy)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/IvyConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${CMAKE_SOURCE_DIR}/cmake/IvyConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/IvyConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Ivy")

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/IvyConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/IvyConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Ivy")
