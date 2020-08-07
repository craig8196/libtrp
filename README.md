

# libtrp
C implementation of The River Protocol (TRP/TRiP, pronounced "trip").
TRiP is a flexible communications protocol.
Unfortunately, libtrip could get confused with a NodeJs library and a website.


## WARNINGS
* STILL BEING DESIGNED!
* The code is experimental and probably has flaws
* Currently in pre-release and will not follow semantic versioning
  until release v1.0.0
* The original author is not a security expert
* Based on UDP and has associated limitations
* If you're going to complain then detail the problem thoroughly
  and have a solution ready (if possible)


## Current Goals
* Easily build custom protocols for niche applications.
* Security by default (user must disable security).
* Persistent, robust connections.
* Data flexibility so behavior, and network behavior, fit the problem space.
* Real-time capabilities (disabled by default for better network performance).
* Framework verbosity to provide information to applications for better
  performance (e.g. User MTU for guaranteed single packet delivery).
* Future-proof, a programming interface and protocol that can be used over
  other mediums and a protocol that can scale up or down with the network.


## Future Goals
* Serializable connections for long disconnects.
* IoT capable through minimal client implementations to minimize build size.


## Why use TRiP?
Let's look at some existing communications protocols...

Note that every protocol suffers from packet attacks due to the insecure
nature of the internet and the Internet Protocol (IP).

TCP suffers from:
* No security by default
* Difficult to implement security
  (causes incorrect implementations or lack of security entirely)
* Slow handshake when doing security
  (one for the connection, one for encryption)
* Head-of-line blocking
  (degrades user and gaming experience in the browser)
* No unreliable send
* No unordered, reliable messaging
* No persistent connections if keep-alive fails
  (can you serialize connection details into a database to resume later?)
* Connection breaks if IPs change or NAT changes
  (this is unfortunate and can disrupt services)
* Dead connections kept alive by some load-balancer configurations
  (creates additional timeouts and error checking by TCP users)
* Heavy-weight solution when managing many connections to the same destination
  (usually to increase throughput)
* Requires another protocol to send messages or multiplex streams
  on the same connection
  (e.g. quirks and limitations of HTTP 1.1/2.0)
* Protocol specific vulnerabilities (e.g. SYN floods)

UDP suffers from:
* No security by default
* Unreliable send only
* Spoofing
* Difficulty in sending large messages/packets
* Connectionless

TRiP suffers from:
* Protocol details handled in user process
  (some extra context switching or system calls)
* Needs another protocol on top
  (that is also part of the design)
* Potentially memory hungry? Will discover and record as needed.
  (struct sizes, tracking, message space pre-allocation)


## Puns
TRIP: TCP Rest In Peace.
TRiPwire: TRiP over physical comms.
TRiPless: TRiP over wireless comms.


## Implementation Notes
Outline of basic design and expectations.


### Testing
The memory and packet interfaces are abstracted to allow for controlled testing.
Memory interface can be adjusted by overwriting the `libtrp_memory.h` header.
Packet interface included with code is for use over UDP.
For testing, a reliable packet interface is used for up to two connections.


### Interface
The interface must be implemented in a generic way.
To/from destinations can be specified using UTF8 or binary strings.
This allows for the interface to remain flexible.


### ABI
The compiled shared object should be backward compatible
taking version and implementation changes into account.
Struct sizes and details should not be made available to the users;
use handles (pointers) to reference needed resources.
Library methods will allocate the needed resources.
For embedded libraries, perhaps an additional header can be made available that
makes common structs globally available for handling one connection at a time.


### Dependencies
* libc: Standard/common functions.
* OS provided UDP socket interface, optional for non-UDP use-cases.
* OS provided event watching (epoll for Linux) for `trip_run` implementation.
* libsodium: For security. Efficient and easy-to-use encryption.
* libcares (in the future): For DNS resolution, optional for direct IP and custom builds.


