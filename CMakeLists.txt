cmake_minimum_required(VERSION 3.4)

set(LIB_SOURCES
	protocol.cpp)
set(LIB_HEADERS
	protocol.hpp)

add_library(udp-discovery STATIC ${LIB_SOURCES} ${LIB_HEADERS})

set(DISCOVERY_TEST_SOURCES
	discovery_test.cpp)
set(DISCOVERY_TEST_LIBS
	udp-discovery)

add_executable(udp-discovery-test ${DISCOVERY_TEST_SOURCES})
target_link_libraries(udp-discovery-test ${DISCOVERY_TEST_LIBS})

set(DISCOVERY_EXAMPLE_SOURCES
	discovery_example.cpp)
set(DISCOVERY_EXAMPLE_LIBS
	udp-discovery)

add_executable(udp-discovery-example ${DISCOVERY_EXAMPLE_SOURCES})
target_link_libraries(udp-discovery-test ${DISCOVERY_TEST_LIBS})
