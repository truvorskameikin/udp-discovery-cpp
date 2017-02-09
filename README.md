# Local network UDP discovery for C++
A small library to add local network discovery feature to your C++ programs with no dependencies

## How to build
This library uses [CMake](https://cmake.org/) to build udp-discovery library and example. To build both library and example one should do:
<pre>
  mkdir build
  cmake ..
  make
</pre>

Also it is possible to just add implementation files to a project and use the build system of that project:
<pre>
  endpoint.cpp
  ip_port.cpp
  protocol.cpp
</pre>

This library has no dependencies.

## How to use

## How to run the example program
[CMake](https://cmake.org/) build of this library produces both static library and example program **udp-discovery-example**. The example program has the following arguments:
<pre>
  Usage: ./udp-discovery-example {discover|discoverable|both} [user_data]

  discover - this instance will have the ability to only discover other instances
  discoverable - this instance will have the ability to only be discovered by other instances
  both - this instance will be able to discover and to be discovered by other instances
  user_data - the string sent when broadcasting, shown next to peer's IP
</pre>

When the example program is run in *discoverable* mode it enters CLI mode waiting for user's commands. The commands are:
<pre>
  > help
  commands are: help, user_data, exit
</pre>