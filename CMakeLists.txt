cmake_minimum_required(VERSION 2.8)

option(BUILD_EXAMPLE "Build example application." OFF)
option(BUILD_TOOL "Build udp-discovery-tool application." OFF)

set(LIB_SOURCES
	udp_discovery_peer.cpp
	udp_discovery_ip_port.cpp
	udp_discovery_protocol.cpp)
set(LIB_HEADERS
	udp_discovery_peer.hpp
	udp_discovery_ip_port.hpp
	udp_discovery_protocol.hpp)

add_library(udp-discovery STATIC ${LIB_SOURCES} ${LIB_HEADERS})

if(BUILD_EXAMPLE OR BUILD_TOOL)
	if(APPLE)
	elseif(UNIX)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
	endif()
endif()

if(BUILD_EXAMPLE)
	set(DISCOVERY_EXAMPLE_SOURCES discovery_example.cpp)
	set(DISCOVERY_EXAMPLE_LIBS udp-discovery)

	if(APPLE)
	elseif(UNIX)
		set(DISCOVERY_EXAMPLE_LIBS ${DISCOVERY_EXAMPLE_LIBS} rt)
	endif()

	if(WIN32)
		set(DISCOVERY_EXAMPLE_LIBS ${DISCOVERY_EXAMPLE_LIBS} Ws2_32)
	endif(WIN32)

	add_executable(udp-discovery-example ${DISCOVERY_EXAMPLE_SOURCES})
	target_link_libraries(udp-discovery-example ${DISCOVERY_EXAMPLE_LIBS})
endif()

if(BUILD_TOOL)
	set(DISCOVERY_TOOL_SOURCES discovery_tool.cpp)
	set(DISCOVERY_TOOL_LIBS udp-discovery)

	if(APPLE)
	elseif(UNIX)
		set(DISCOVERY_TOOL_LIBS ${DISCOVERY_TOOL_LIBS} rt)
	endif()

	add_executable(udp-discovery-tool ${DISCOVERY_TOOL_SOURCES})
	target_link_libraries(udp-discovery-tool ${DISCOVERY_TOOL_LIBS})
endif()
