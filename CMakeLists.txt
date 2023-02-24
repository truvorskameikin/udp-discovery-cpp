cmake_minimum_required(VERSION 3.0)

project(udp-discovery-cpp)

option(BUILD_EXAMPLE "Build example application." OFF)
option(BUILD_TOOL "Build udp-discovery-tool application." OFF)
option(BUILD_TEST "Build test." ON)

set(LIB_SOURCES
	udp_discovery_ip_port.cpp
	udp_discovery_peer.cpp
	udp_discovery_protocol.cpp)
set(LIB_HEADERS
	udp_discovery_ip_port.hpp
	udp_discovery_peer.hpp
	udp_discovery_peer_parameters.hpp
	udp_discovery_protocol.hpp
	udp_discovery_protocol_version.hpp)

add_library(udp-discovery STATIC ${LIB_SOURCES} ${LIB_HEADERS})
set_property(TARGET udp-discovery PROPERTY CXX_STANDARD 98)

if(UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
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
	set_property(TARGET udp-discovery-example PROPERTY CXX_STANDARD 98)
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
	set_property(TARGET udp-discovery-example PROPERTY CXX_STANDARD 98)
endif()

if(BUILD_TEST)
	enable_testing()

	add_executable(udp-discovery-protocol-test udp_discovery_protocol.cpp udp_discovery_protocol_test.cpp)
	set_property(TARGET udp-discovery-protocol-test PROPERTY CXX_STANDARD 98)
	add_test(udp-discovery-protocol-test udp-discovery-protocol-test)

	add_executable(udp-discovery-uuid-test udp_discovery_uuid_test.cpp)
	set_property(TARGET udp-discovery-uuid-test PROPERTY CXX_STANDARD 98)
	add_test(udp-discovery-uuid-test udp-discovery-uuid-test)

	add_executable(udp-discovery-peer-e2e-test udp_discovery_protocol.cpp udp_discovery_peer.cpp udp_discovery_peer_e2e_test.cpp)
	set_property(TARGET udp-discovery-peer-e2e-test PROPERTY CXX_STANDARD 98)
	add_test(udp-discovery-peer-e2e-test udp-discovery-peer-e2e-test)
endif()
