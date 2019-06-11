# Local network UDP discovery for C++ [![Build Status](https://travis-ci.org/truvorskameikin/udp-discovery-cpp.svg?branch=develop)](https://travis-ci.org/truvorskameikin/udp-discovery-cpp)

A small library to add local network discovery feature to your C++ programs with no dependencies

## How to build
This library uses [CMake](https://cmake.org/) to build static library, examples and tools. To build all targets do:
<pre>
cd udp-discovery-cpp
mkdir build
cd build
cmake -DBUILD_EXAMPLE=ON -DBUILD_TOOL=ON ..
make
</pre>

Also it is possible to just add implementation files to a project and use the build system of that project:
<pre>
udp_discovery_peer.cpp
udp_discovery_ip_port.cpp
udp_discovery_protocol.cpp
</pre>

This library has no dependencies.

## How to use

## How to run the example program and a discovery tool
[CMake](https://cmake.org/) build of this library produces static library, example program **udp-discovery-example** and a tool to discover local peers **udp-discovery-tool**.

### The example program

The example program **udp-discovery-example** has the following arguments:
<pre>
Usage: ./udp-discovery-example {discover|discoverable|both} [user_data]

discover - this instance will have the ability to only discover other instances
discoverable - this instance will have the ability to only be discovered by other instances
both - this instance will be able to discover and to be discovered by other instances
user_data - the string sent when broadcasting, shown next to peer's IP
</pre>

When the example program is run in *discoverable* mode it enters CLI mode waiting for user's commands:
<pre>
> help
commands are: help, user_data, exit
</pre>

### Discovery tool

The discovery tool **udp-discovery-tool** has the following arguments:
<pre>
Usage: ./udp-discovery-tool application_id port
  application_id - integer id of application to discover
  port - port used by application
</pre>
