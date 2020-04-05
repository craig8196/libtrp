
- [x] Test memory interface.
- [x] Create test framework from ss?
- [x] Setup for .so only to start, static library requires special setup for libsodium.
- [x] Buffer structure:
struct tripbuf_s
{
    size_t cap;
    size_t len;
    //char buf[];
};
tripbuf_len(buf);
tripbuf_get(buf);
- [x] Serialization. Use pack/unpack from beejs guide!!!
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

