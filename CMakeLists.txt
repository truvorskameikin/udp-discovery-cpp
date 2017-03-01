cmake_minimum_required(VERSION 2.8)

set(LIB_SOURCES
	endpoint.cpp
	ip_port.cpp
	protocol.cpp)
set(LIB_HEADERS
	endpoint.hpp
	ip_port.hpp
	protocol.hpp)

add_library(udp-discovery STATIC ${LIB_SOURCES} ${LIB_HEADERS})

set(DISCOVERY_EXAMPLE_SOURCES
	discovery_example.cpp)
set(DISCOVERY_EXAMPLE_LIBS
	udp-discovery)

set(DISCOVERY_TOOL_SOURCES
	discovery_tool.cpp)
set(DISCOVERY_TOOL_LIBS
	udp-discovery)

if(WIN32)
set(DISCOVERY_EXAMPLE_LIBS ${DISCOVERY_EXAMPLE_LIBS}
	Ws2_32)
endif(WIN32)

if(APPLE)
elseif(UNIX)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

if(APPLE)
elseif(UNIX)
set(DISCOVERY_EXAMPLE_LIBS ${DISCOVERY_EXAMPLE_LIBS}
	rt)
set(DISCOVERY_TOOL_LIBS ${DISCOVERY_TOOL_LIBS}
	rt)
endif()

add_executable(udp-discovery-example ${DISCOVERY_EXAMPLE_SOURCES})
target_link_libraries(udp-discovery-example ${DISCOVERY_EXAMPLE_LIBS})

add_executable(udp-discovery-tool ${DISCOVERY_TOOL_SOURCES})
target_link_libraries(udp-discovery-tool ${DISCOVERY_TOOL_LIBS})
