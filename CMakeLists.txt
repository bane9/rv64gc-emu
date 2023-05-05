cmake_minimum_required(VERSION 3.18.4)
project(rv64gc_emu)

if (USE_EMSCRIPTEN)
  if (NOT DEFINED EM_LOAD_PATH)
    message(ERROR "You must specify -DEM_LOAD_PATH as an argument. Please refer to the Emscripten instructions in Readme")
  endif()

  set(NATIVE_CLI 1)
endif()

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
  
  source/peripherals/clint.cpp
  source/peripherals/plic.cpp
  source/peripherals/ram.cpp
  source/peripherals/virtio.cpp
  
  source/instructions/loadinsn.cpp
  source/instructions/otherinsn.cpp
  source/instructions/itypeinsn.cpp
  source/instructions/btypeinsn.cpp
  source/instructions/i64insn.cpp
  source/instructions/r64insn.cpp
  source/instructions/rtypeinsn.cpp
  source/instructions/stypeinsn.cpp
  source/instructions/csrtypeinsn.cpp
  source/instructions/atomictype.cpp
  source/instructions/fdtypeinsn.cpp
  source/instructions/ctypeinsn.cpp
)

set(SRC_FILES_MAIN
  ${SRC_FILES_COMMON}
  main.cpp
)

if (${NATIVE_CLI})
set(SRC_FILES_MAIN
  ${SRC_FILES_MAIN}
  source/peripherals/native_cli.cpp
)
else()
set(SRC_FILES_MAIN
  ${SRC_FILES_MAIN}
  source/peripherals/gpu.cpp
)
endif()

add_executable(rv64gc_emu "${SRC_FILES_MAIN}")

set_property(TARGET rv64gc_emu PROPERTY CXX_STANDARD 20)
set_property(TARGET rv64gc_emu PROPERTY C_STANDARD 17)

if (USE_EMSCRIPTEN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -matomics -mbulk-memory")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -matomics -mbulk-memory")
  set_target_properties(rv64gc_emu PROPERTIES LINK_FLAGS "--preload-file ${EM_LOAD_PATH} -s ALLOW_MEMORY_GROWTH=1 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(OPTIMIZATION_FLAG "-Og")
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
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(rv64gc_emu PRIVATE Threads::Threads)

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
  source/peripherals/native_cli.cpp
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
