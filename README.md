# Local network UDP discovery for C++ [![Build Status](https://travis-ci.org/truvorskameikin/udp-discovery-cpp.svg?branch=develop)](https://travis-ci.org/truvorskameikin/udp-discovery-cpp)

A small library to add local network discovery feature to your C++ programs with no dependencies

## Users

For the list of **udp-discovery-cpp** library users please refer to [USERS.md](USERS.md) document.

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

The example program **udp-discovery-example** can be a very good reference on how to use this library. It uses 12021 as port and 7681412 as application id, these values users of *udp-discovery-cpp* library should decide on.

To start the discovery peer the users should first create *udpdiscovery::PeerParameters* object and fill in the parameters:
```cpp
udpdiscovery::PeerParameters parameters;

// Sets the port that will be used for receiving and sending discovery packets.
parameters.set_port(kPort);

// Sets the application id, only peers with the same application id can be discovered.
parameters.set_application_id(kApplicationId);

// This peer can discover other peers.
parameters.set_can_discover(true);

// This peer can be discovered by other peers.
parameters.set_can_be_discovered(true);

// Users can tweak other parameters (timeouts, peer comparison mode) to fit their needs.
// Please refer to udp_discovery_peer_parameters.hpp file.
```

Then create a *udpdiscovery::Peer* object and start discovery providing user data that will be associated with this peer:
```cpp
udpdiscovery::Peer peer;
peer.Start(parameters, user_data);
```

User data will be transfered and will be discovered by other peers. User data can be user by user application to store some meaningful data that application wants to share between peers.

The created and started *udpdiscovery::Peer* object can be used to list currently discovered peers:
```cpp
std::list<udpdiscovery::DiscoveredPeer> new_discovered_peers = peer.ListDiscovered();
```

There are two options to compare discovered peers and to consider them as equal:
* *kSamePeerIp* - compares only ip part of the received discovery packet, so multiple instances of application sending packets from the same ip will be considered as one peer.
* *kSamePeerIpAndPort* - the default value, compares both ip and port of the received discovery packet, so multiple instances of application sending packets from the same ip will be considered as different peers.

Users can use *udpdiscovery::Same* function to compare two lists of discovered peers to decide if the list of discovered peers is the same or new peers appear or some peers disappear:
```cpp
bool is_same = udpdiscovery::Same(parameters.same_peer_mode(), discovered_peers, new_discovered_peers);
```

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

*user_data* command changes user data associated with this peer.
*exit* command exits the **udp-discovery-example** gracefully sending *kPacketIAmOutOfHere* packet.

### Discovery tool

The discovery tool **udp-discovery-tool** has the following arguments:
<pre>
Usage: ./udp-discovery-tool application_id port
  application_id - integer id of application to discover
  port - port used by application
</pre>
