cmake_minimum_required(VERSION 3.26)

project(client
        VERSION 0.2.1
        DESCRIPTION ""
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(SANITIZE TRUE)

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

set(SOURCE_LIST
#        ${SOURCE_DIR}/networking.cpp
)
SET(SOURCE_MAIN ${SOURCE_DIR}/main.cpp
                ${SOURCE_DIR}/networking.cpp
                ${SOURCE_DIR}/transfer.cpp)
set(HEADER_LIST
        ${INCLUDE_DIR}/networking.hpp
        ${INCLUDE_DIR}/transfer.hpp
)

include_directories(${INCLUDE_DIR})

add_executable(client ${SOURCE_MAIN} ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(client PRIVATE include)
target_include_directories(client PRIVATE /usr/local/include)
target_link_directories(client PRIVATE /usr/local/lib)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_include_directories(client PRIVATE /usr/include)
endif ()

set_target_properties(client PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})

find_library(LIBDC_ERROR p101_error REQUIRED)
find_library(LIBDC_ENV p101_env REQUIRED)
find_library(LIBDC_FSM p101_fsm REQUIRED)

target_link_libraries(client PUBLIC ${LIBDC_ERROR})
target_link_libraries(client PUBLIC ${LIBDC_ENV})
target_link_libraries(client PUBLIC ${LIBDC_FSM})
# Add compiler flags
target_compile_options(client PRIVATE
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
        target_compile_options(client PRIVATE
                -fsanitize=address
                -fsanitize=undefined
        )
        target_link_libraries(client PRIVATE
                -fsanitize=address
                -fsanitize=undefined
        )
    else ()
        message(WARNING "Sanitizer checks are not supported with this compiler.")
    endif ()
endif ()

set_target_properties(client PROPERTIES OUTPUT_NAME "client")
install(TARGETS client DESTINATION bin)
