# Users of udp-discovery-cpp

This file lists all known users of **udp-discovery-cpp** library. Feel free to add your project here filing a [pull request](https://github.com/truvorskameikin/udp-discovery-cpp/pulls)!

#### Table of content

- [Smotreshka (prototype, not in production)](#smotreshka)
- [Retroshare](#retroshare)
- [Xeres](#xeres)

<a name="smotreshka"/>

## Smotreshka (prototype, not in production)

[Smotreshka](https://smotreshka.tv/) used **udp-discovery-cpp in** the prototype that showed the ability to discover user devices in the same home network that are not currently logged in. The user devices talked to each other using HTTP protocol (simple HTTP server was built into every instance of Smotreshka) and used **udp-dicovery-cpp** for discovering devices' IPs. User data field was used to store some [JSON](https://www.json.org/) metadata describing the device.

<a name="retroshare"/>

## Retroshare

[Retroshare](https://retroshare.cc/) uses **udp-discovery-cpp** to find its peers on the LAN. This improves the connectivity.

<a name="xeres"/>

## Xeres

[Xeres](https://xeres.io/) is compatible with Retroshare and, while it doesn't directly use **udp-discovery-cpp**, it has a reimplementation of it and intends to stay compatible with the protocol. It uses it to find peers on the LAN.
