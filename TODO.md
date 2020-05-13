
- [x] Test memory interface.
- [x] Create test framework from ss?
- [x] Setup for .so only to start, static library requires special setup for libsodium.
- [x] Buffer structure:
- [x] Serialization. Use pack/unpack from beejs guide!!!

- [ ] Make connection IDs uint64_t.

- [ ] Non-UDP packet framework just to connect two routers "0" or "1".

- [ ] Connection data structure, resizable array.
- [ ] Stream data structure, resizable array.
- [ ] Message Descriptor structure.
- [ ] Message structure simplification with message descriptor.
- [ ] Zone data structure. Mostly conceptual?
- [ ] Incoming message data structure.

- [ ] Client library routines for simplification?
- [ ] IP:Port to index.
- [ ] Create compilable framework.
- [ ] Determine next steps with packet interface.
- [ ] Make packet interface close to UDP interface.
- [ ] Make reporting buffer size and errors better.
- [ ] Make packet interface async.
- [ ] Make trip calls from packet interface with functions.
- [ ] Send OPEN
- [ ] Send CHAL
- [ ] Send PING
- [ ] Send NOTI
- [ ] Send DISC
- [ ] Send DATA


## Static Compilation Discussion
So libsodium needs to be compiled with -fpic or something like that.
Basically, need a global offset table entry to get to compile static.
Alternatively, downloading original source of libsodium gives flexibility
to turn off -fpic flag.

