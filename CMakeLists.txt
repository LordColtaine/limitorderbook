cmake_minimum_required(VERSION 3.15)
project(LimitOrderBook CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Helps VSCode IntelliSense

# --- 1. Manually find the PCAP header and library files ---
find_path(PCAP_INCLUDE_DIR pcap.h)
find_library(PCAP_LIBRARY pcap)

if(NOT PCAP_INCLUDE_DIR OR NOT PCAP_LIBRARY)
    message(FATAL_ERROR "Failed to find libpcap. Ensure 'sudo apt install libpcap-dev' was run.")
endif()

add_executable(program 
                src/main.cpp
                src/limitorderbook.cpp
                src/orderpool.cpp
                src/ringbuffer.cpp)

target_include_directories(program PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_include_directories(program PRIVATE ${PCAP_INCLUDE_DIR})
target_link_libraries(program PRIVATE ${PCAP_LIBRARY})