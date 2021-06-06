#!/bin/sh

script_dir=`dirname $0`
clang-format -i --style=Google ${script_dir}/udp_discovery_protocol.hpp ${script_dir}/udp_discovery_protocol.cpp ${script_dir}/udp_discovery_protocol_test.cpp
