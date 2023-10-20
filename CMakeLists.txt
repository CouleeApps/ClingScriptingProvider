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

find_path(CLING_LIB_DIR
        NAMES cling.dll libcling.dylib libcling.so
        PATHS ${CLING_PATH}/lib
        REQUIRED
)

find_library(CLING_LIB
        NAMES cling libcling
        PATHS ${CLING_PATH}/lib
        REQUIRED
)

find_path(CLING_INCLUDE_DIR
        NAMES cling
        PATHS ${CLING_PATH}/include
        REQUIRED
)

message(STATUS "Found libcling: ${CLING_LIB}")
message(STATUS "Found cling headers: ${CLING_INCLUDE_DIR}")

find_path(
        BN_API_PATH
        NAMES binaryninjaapi.h
        HINTS binaryninjaapi ../binaryninja/api $ENV{BN_API_PATH}
        REQUIRED
)
message(STATUS "Found binary ninja headers: ${BN_API_PATH}")

add_subdirectory(${BN_API_PATH} api)

add_library(ClingScriptingProvider SHARED ${FILES})

target_include_directories(ClingScriptingProvider PUBLIC ${CLING_INCLUDE_DIR})
target_link_libraries(ClingScriptingProvider PUBLIC ${CLING_LIB})
target_link_directories(ClingScriptingProvider PUBLIC ${CLING_LIB_DIR})

target_link_libraries(ClingScriptingProvider PUBLIC binaryninjaapi)

bn_install_plugin(ClingScriptingProvider)
