cmake_minimum_required(VERSION 3.26)

project(ClingScriptingProvider CXX)

set(CMAKE_CXX_STANDARD 17)
set(HEADLESS 1)

set(FILES
        library.h
        library.cpp
        ClingScriptingProvider.h
        ClingScriptingProvider.cpp
)

set(CLING_PATH "CLING_PATH-NOTFOUND" CACHE STRING "Path to cling install")
list(APPEND CMAKE_PREFIX_PATH ${CLING_PATH}/lib/cmake/cling)
list(APPEND CMAKE_PREFIX_PATH ${CLING_PATH}/lib/cmake/clang)

find_package(Cling REQUIRED CONFIG HINTS "")

find_path(
        BN_API_PATH
        NAMES binaryninjaapi.h
        HINTS binaryninjaapi ../binaryninja/api $ENV{BN_API_PATH}
        REQUIRED
)
message(STATUS "Found binary ninja headers: ${BN_API_PATH}")

add_subdirectory(${BN_API_PATH} api)

add_library(ClingScriptingProvider SHARED ${FILES})

target_link_libraries(ClingScriptingProvider PUBLIC
        clangFrontendTool
        clingInterpreter
        clingMetaProcessor
        clingUserInterface
        clingUtils
        LLVMSupport
)
target_include_directories(ClingScriptingProvider PUBLIC ${CLING_INCLUDE_DIRS})
target_link_libraries(ClingScriptingProvider PUBLIC binaryninjaapi)

if (NOT ${LLVM_ENABLE_RTTI})
    target_compile_options(ClingScriptingProvider PUBLIC -fno-rtti)
endif()

set_target_properties(ClingScriptingProvider PROPERTIES
        CXX_VISIBILITY_PRESET default
        CXX_STANDARD_REQUIRED ON
        VISIBILITY_INLINES_HIDDEN OFF
        POSITION_INDEPENDENT_CODE ON
        ENABLE_EXPORTS 1
        INSTALL_RPATH ${LLVM_LIBRARY_DIRS})

bn_install_plugin(ClingScriptingProvider)
