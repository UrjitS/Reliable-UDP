cmake_minimum_required(VERSION 3.26)

project(fsm_network_share_client
        VERSION 0.2.1
        DESCRIPTION ""
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(SANITIZE TRUE)

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

set(SOURCE_LIST
        ${SOURCE_DIR}/file_transfer.cpp
        ${SOURCE_DIR}/networking.cpp
)
SET(SOURCE_MAIN ${SOURCE_DIR}/main.cpp)
set(HEADER_LIST
        ${INCLUDE_DIR}/file_transfer.hpp
        ${INCLUDE_DIR}/networking.hpp
)

include_directories(${INCLUDE_DIR})

add_executable(fsm_network_share_client ${SOURCE_MAIN} ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(fsm_network_share_client PRIVATE include)
target_include_directories(fsm_network_share_client PRIVATE /usr/local/include)
target_link_directories(fsm_network_share_client PRIVATE /usr/local/lib)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_include_directories(fsm_network_share_client PRIVATE /usr/include)
endif ()

set_target_properties(fsm_network_share_client PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})

find_library(LIBDC_ERROR dc_error REQUIRED)
find_library(LIBDC_ENV dc_env REQUIRED)
find_library(LIBDC_C dc_c REQUIRED)
find_library(LIBDC_POSIX dc_posix REQUIRED)
find_library(LIBDC_UNIX dc_unix REQUIRED)
find_library(LIBDC_UTIL dc_util REQUIRED)
find_library(LIBDC_FSM dc_fsm REQUIRED)
find_library(LIB_CONFIG config REQUIRED)
find_library(LIBDC_APPLICATION dc_application REQUIRED)

target_link_libraries(fsm_network_share_client PUBLIC ${LIBDC_ERROR})
target_link_libraries(fsm_network_share_client PUBLIC ${LIBDC_ENV})
target_link_libraries(fsm_network_share_client PUBLIC ${LIBDC_C})
target_link_libraries(fsm_network_share_client PUBLIC ${LIBDC_POSIX})
target_link_libraries(fsm_network_share_client PUBLIC ${LIBDC_UNIX})
target_link_libraries(fsm_network_share_client PUBLIC ${LIBDC_UTIL})
target_link_libraries(fsm_network_share_client PUBLIC ${LIBDC_FSM})
target_link_libraries(fsm_network_share_client PUBLIC ${LIB_CONFIG})
target_link_libraries(fsm_network_share_client PUBLIC ${LIBDC_APPLICATION})

# Add compiler flags
target_compile_options(fsm_network_share_client PRIVATE
        -Wall              # Enable all compiler warnings
        -Wextra            # Enable extra compiler warnings
        -pedantic          # Enable pedantic mode
        -O2                # Optimization level 2
        -g                 # Generate debug information
        -fPIC              # Generate position-independent code
        # Add more flags as needed
)

# Add sanitizer checks
if (SANITIZE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(fsm_network_share_client PRIVATE
                -fsanitize=address
                -fsanitize=undefined
        )
        target_link_libraries(fsm_network_share_client PRIVATE
                -fsanitize=address
                -fsanitize=undefined
        )
    else ()
        message(WARNING "Sanitizer checks are not supported with this compiler.")
    endif ()
endif ()

set_target_properties(fsm_network_share_client PROPERTIES OUTPUT_NAME "client")
install(TARGETS fsm_network_share_client DESTINATION bin)
