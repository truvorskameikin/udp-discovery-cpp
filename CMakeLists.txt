cmake_minimum_required(VERSION 3.4)

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

if(WIN32)
set(DISCOVERY_EXAMPLE_LIBS ${DISCOVERY_EXAMPLE_LIBS}
	Ws2_32)
endif(WIN32)

add_executable(udp-discovery-example ${DISCOVERY_EXAMPLE_SOURCES})
target_link_libraries(udp-discovery-example ${DISCOVERY_EXAMPLE_LIBS})
