cmake_minimum_required(VERSION 3.22.1)

project(rv64gc_emu)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FetchContent)

set(INCLUDE_DIRS
 ${PROJECT_SOURCE_DIR}/source/include/
 ${PROJECT_SOURCE_DIR}/source/instructions/include/
 ${PROJECT_SOURCE_DIR}/source/peripherals/include/
)

set(SRC_FILES_COMMON
  source/bus.cpp
  source/decoder.cpp
  source/helper.cpp
  source/cpu.cpp
  source/csr.cpp
  source/interrupt.cpp
  source/mmu.cpp
  source/misc.cpp
  
  source/peripherals/src/clint.cpp
  source/peripherals/src/plic.cpp
  source/peripherals/src/ram.cpp
  
  source/instructions/src/loadinsn.cpp
  source/instructions/src/otherinsn.cpp
  source/instructions/src/itypeinsn.cpp
  source/instructions/src/btypeinsn.cpp
  source/instructions/src/i64insn.cpp
  source/instructions/src/r64insn.cpp
  source/instructions/src/rtypeinsn.cpp
  source/instructions/src/stypeinsn.cpp
  source/instructions/src/csrtypeinsn.cpp
  source/instructions/src/atomictype.cpp
  source/instructions/src/fdtypeinsn.cpp
  source/instructions/src/ctypeinsn.cpp
)

set(SRC_FILES_MAIN
  ${SRC_FILES_COMMON}
  main.cpp
)

if (${NATIVE_CLI})
set(SRC_FILES_MAIN
  ${SRC_FILES_MAIN}
  source/peripherals/src/native_cli.cpp
)
else()
set(SRC_FILES_MAIN
  ${SRC_FILES_MAIN}
  source/peripherals/src/gpu.cpp
)
endif()

add_executable(rv64gc_emu "${SRC_FILES_MAIN}")

set_property(TARGET rv64gc_emu PROPERTY CXX_STANDARD 20)
set_property(TARGET rv64gc_emu PROPERTY C_STANDARD 17)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(OPTIMIZATION_FLAG "-O0")
  target_compile_options(rv64gc_emu PRIVATE -fsanitize=address)
  target_link_options(rv64gc_emu PRIVATE -fsanitize=address)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(OPTIMIZATION_FLAG "-O3")
  set_property(TARGET rv64gc_emu PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_compile_options(rv64gc_emu PRIVATE ${OPTIMIZATION_FLAG})

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
  GIT_PROGRESS    TRUE
)

FetchContent_MakeAvailable(fmt)

if (${NATIVE_CLI})
  target_include_directories(rv64gc_emu PRIVATE ${INCLUDE_DIRS})

  target_link_libraries(rv64gc_emu PRIVATE fmt::fmt)

  add_compile_definitions(NATIVE_CLI=1)
else()
  find_package(SDL2 REQUIRED)
  find_package(SDL2_TTF REQUIRED)

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(VTERM REQUIRED IMPORTED_TARGET vterm)

  find_package(ICU REQUIRED COMPONENTS uc i18n)

  set(ALL_INCLUDE_DIRS
    ${INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS} 
    ${VTERM_INCLUDE_DIRS} 
    ${SDL2_TTF_INCLUDE_DIRS} 
    ${ICU_INCLUDE_DIRS}
  )

  target_include_directories(rv64gc_emu PRIVATE ${ALL_INCLUDE_DIRS})

  target_link_libraries(rv64gc_emu PRIVATE fmt::fmt SDL2::SDL2 PkgConfig::VTERM SDL2_ttf ${ICU_LIBRARIES})
endif()

# Tests

enable_testing()

set(SRC_FILES_TEST
  ${SRC_FILES_COMMON}
  tests/test_cpu.cpp
  source/peripherals/src/native_cli.cpp
)

add_executable(test_cpu "${SRC_FILES_TEST}")

set_property(TARGET test_cpu PROPERTY CXX_STANDARD 20)
set_property(TARGET test_cpu PROPERTY C_STANDARD 17)

target_include_directories(test_cpu PRIVATE ${INCLUDE_DIRS} tests/)
target_link_libraries(test_cpu PRIVATE fmt::fmt)

target_compile_definitions(test_cpu PRIVATE CPU_TEST=1 CPU_VERBOSE_DEBUG=1)

target_compile_options(test_cpu PRIVATE -fsanitize=address)
target_link_options(test_cpu PRIVATE -fsanitize=address)

add_test(
 NAME rv64ui_test
 COMMAND $<TARGET_FILE:test_cpu> "../testbins/rv64ui/bin/"
)

add_test(
 NAME rv64um_test
 COMMAND $<TARGET_FILE:test_cpu> "../testbins/rv64um/bin/"
)

add_test(
  NAME rv64ua_test
  COMMAND $<TARGET_FILE:test_cpu> "../testbins/rv64ua/bin/"
)

add_test(
  NAME rv64uf_test
  COMMAND $<TARGET_FILE:test_cpu> "../testbins/rv64uf/bin/"
)

add_test(
  NAME rv64ud_test
  COMMAND $<TARGET_FILE:test_cpu> "../testbins/rv64ud/bin/"
)

if (NOT(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64"))
  message(WARNING "F and D tests may fail due to way some platforms set FE_INEXACT flag after fpu calculations\n")
endif()

add_test(
  NAME rv64uc_test
  COMMAND $<TARGET_FILE:test_cpu> "../testbins/rv64uc/bin/"
)

add_test(
  NAME rv64mi_test
  COMMAND $<TARGET_FILE:test_cpu> "../testbins/rv64mi/bin/"
)

add_test(
  NAME rv64si_test
  COMMAND $<TARGET_FILE:test_cpu> "../testbins/rv64si/bin/"
)
