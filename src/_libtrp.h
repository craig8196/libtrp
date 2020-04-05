/*******************************************************************************
 * Copyright (c) 2019 Craig Jacobson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/
/**
 * @file _libtrp.h
 * @author Craig Jacobson
 * @brief TRiP internal implementation.
 *
 * Priority is synonymous with real-time, or as real-time as we can get.
 */
#ifndef _LIBTRP_H_
#define _LIBTRP_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <sodium.h>


#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif


#define trip_routerify(R, _R) _trip_router_t * R  = ((_trip_router_t *)(_R));
#define trip_connify(C, _C) _trip_connection_t * C  = ((_trip_connection_t *)(_C));
#define trip_streamify(S, _S) _trip_stream_t * S  = ((_trip_stream_t *)(_S));
#define trip_bufferify(B, _B) _tripbuf_t * B  = ((_tripbuf_t *)(_B));


/* Shorten some cryptography calls. */
#define _TRIP_NONCE (crypto_box_NONCEBYTES)
#define _TRIP_BOX_PAD (crypto_box_NONCEBYTES)
#define _trip_nonce_init(nonce) randombytes_buf((nonce), crypto_box_NONCEBYTES)
#define _trip_sign crypto_sign_detached
#define _trip_unsign crypto_sign_verify_detached
#define _trip_shared_key crypto_box_beforenm
#define _trip_box crypto_box_easy_afternm
#define _trip_unbox crypto_box_open_easy_afternm
#define _trip_seal crypto_box_seal
#define _trip_unseal crypto_box_seal_open


enum _tripr_state
{
    _TRIPR_STATE_START,
    _TRIPR_STATE_STOP,
    _TRIPR_STATE_BIND,
    _TRIPR_STATE_LISTEN,
    _TRIPR_STATE_NOTIFY,
    _TRIPR_STATE_CLOSE,
    _TRIPR_STATE_UNBIND,
    _TRIPR_STATE_END,
    _TRIPR_STATE_ERROR,
};

enum _trip_control
{
    _TRIP_CONTROL_OPEN,
};

typedef struct _trip_router_s _trip_router_t;
typedef struct _trip_connection_s _trip_connection_t;
typedef struct _trip_stream_s _trip_stream_t;
typedef struct _trip_msg_s _trip_msg_t;
typedef struct _tripbuf_s _tripbuf_t;

typedef struct _trip_wait_s _trip_wait_t;

typedef struct _trip_prefix_s _trip_prefix_t;
typedef struct _trip_open_s _trip_open_t;

#define _TRIP_MAX_EVENTS (16)
struct _trip_wait_s
{
    int efd;
    int timeout;
};

#define _TRIP_ALLOW_IN         (1 << 0)
#define _TRIP_ALLOW_OUT        (1 << 1)
#define _TRIP_ALLOW_PLAINOPEN  (1 << 2)
#define _TRIP_ALLOW_PLAINSEGM  (1 << 3)
#define _TRIP_ALLOW_PLAINSIGN  (1 << 4)

struct _trip_router_s
{
    /* Frequently Accessed */
    void *ud;

    /* User Hooks */
    trip_handle_screen_t *screen;
    trip_handle_watch_t *watch;
    trip_handle_timeout_t *timeout;
    trip_handle_connection_t *connection;
    trip_handle_stream_t *stream;
    trip_handle_message_t *message;

    /* Wait Data */
    _trip_wait_t *wait;
    int wtimeout;

    /* State */
    enum _tripr_state state;
    uint64_t statedeadline;

    /* Error Handling */
    int error;
    char *errmsg;

    /* Connection Info
     * An array of pointers is currently used.
     * The connection's ID is also the index when modulo conlen.
     * This gives us lookup speed and some level of randomness.
     * Also, guarantee's ID uniqueness within the array.
     * Example:
     * conmask = 0x03;
     * concap = 4;
     * id = 0xABCDEF11; // upper bits can be random if above max_conn
     * true = id % concap == id & conmask == index == 1;
     */
    uint32_t conmask; /* concap - 1 */
    uint32_t concap; /* Power of 2. */
    uint32_t conlen; /* Number of connections. */
    _trip_connection_t **con; /* Array of connections. */
    _trip_connection_t *confree; /* Linked list of previously used connections. */
    _trip_connection_t **conempty; /* Pointer to where to start searching. */

    /* Send Management */
    _trip_connection_t *sendbeg;
    _trip_connection_t *sendend;
    _tripbuf_t *sendbuf;
    _tripbuf_t *decryptbuf;

    /* Memory, Packet, and Encryption */
    trip_memory_t *mem;
    trip_packet_t *pack;
    unsigned char *openpub;
    unsigned char *opensec;
    unsigned char *signpub;
    unsigned char *signsec;

    /* Limits */
    uint32_t max_mem;
    uint32_t max_packet_mem;
    uint32_t max_conn;// TODO use uppermost bits on max_conn for connection ID randomization??
    uint32_t max_in;
    uint32_t max_out;
    uint32_t max_packet_read_count;
    uint32_t max_connection_send;
    uint32_t max_message_id;

    /* Settings */
    // TODO compress to bit field
    uint32_t allow;
    bool allow_in;
    bool allow_out;
    bool allow_plain_open;
    bool allow_plain_seg;
    bool allow_plain_sig;

    int timeout_bind;
    int timeout_data;
    int timeout_close;
    int timeout_stop;
    int timeout_notify;
};

enum _trip_message_q
{
    _TRIP_MESSAGE_PRIORITY = 0,
    _TRIP_MESSAGE_WHENEVER = 1,
};

enum _tripc_state
{
    _TRIPC_STATE_START,
    _TRIPC_STATE_OPEN,
    _TRIPC_STATE_CHAL,
    _TRIPC_STATE_PING,
    _TRIPC_STATE_READY,
    _TRIPC_STATE_READY_PING,
    _TRIPC_STATE_NOTIFY,
    _TRIPC_STATE_DISCONNECT,
    _TRIPC_STATE_END,
    _TRIPC_STATE_ERROR,
};

struct _trip_connection_s
{
    /* Frequently Accessed */
    void *ud;
    _trip_router_t *router;

    /* Address */
    size_t infolen;
    unsigned char *info;

    /* Error */
    int error;
    char *msg;

    uint32_t id;
    /* Used to round-robin through connections when sending data.
     * Re-used to link unused connection structs.
     */
    _trip_connection_t *next;

    /* State */
    enum _tripc_state state;
    uint64_t statedeadline;


    /* Sometimes we're unable to send the buffer, store here until ready.
     * segfull is true if just waiting to send.
     * segment contains the final product.
     * segwork contains unencrypted portion.
     */
    bool segfull;
    _tripbuf_t *segment;
    _tripbuf_t *segwork;

    /* Message Queues
     * Focus on sending one message at a time.
     * Only send partial buffers if priority and nothing else will fit.
     * Partial buffers will flush after max_message_buffer_wait ms.
     * Resending messages, or partial messages,
     * jumps to the front of the appropriate Q.
     * 0 - Priority Q.
     * 1 - Whenever Q.
     * 
     * Pull next partial message from Whenever Q when which indicator reaches zero.
     * This helps avoid starvation.
     * sendwhich & sendmask > 0 => priority next
     * sendwhich & sendmask = 0 => whenever next
     *
     * If sendmask = 0x01, then queueing is fair.
     * If sendmask = 0x03, then priority sends first 3/4 of time, whenever 1/4.
     */
    uint32_t sendmask;
    uint32_t sendwhich;
    _trip_msg_t *sendbeg[2];
    _trip_msg_t *sendend[2];
    uint32_t nextmsgid;
    int zone;
};

/* First options/flags are set by the user.
 * Remaining flags are set internally and used by the framework.
 */
#define _TRIPS_OPT_BACKFLOW (1 << 4)
#define _TRIPS_OPT_CLOSED   (1 << 5)
#define _TRIPS_OPT_STALLED  (1 << 6)
#define _TRIPS_OPT_PUBMASK (0x000F)
#define _TRIPS_OPT_SECMASK (0x007F)

// TODO idea for stream message lookup is to store in n x n array
// TODO n long and n max messages linked per entry grow array by one
// TODO attackers would need number that hashes perfectly and growing
// TODO will throw clustering off
struct _trip_stream_s
{
    /* Frequently Accessed */
    void *ud;
    _trip_connection_t *connection;

    int id;
    int flags;
    _trip_msg_t *listbeg;
    _trip_msg_t *listend;
};

struct _trip_msg_s
{
    _trip_stream_t *stream;

    /* Streams are stored in a list.
     */
    _trip_msg_t *next;

    int zone;// 0 or 1, indicates which segment zone this message falls
    uint32_t id;// id or index in zone
    int priority;// 0 high 1 low
    size_t boff; // amount sent
    size_t blen; // length, don't change
    unsigned char *buf; // data, don't change
    _trip_msg_t *sendnext; // next message in the queue
};

struct _tripbuf_s
{
    int cap;
    int len;
};

struct _trip_prefix_s
{
    bool encrypted;
    int control;
    uint32_t id;
    uint32_t seq;
};

struct _trip_open_s
{
    uint16_t version;
    uint32_t route_len;
    unsigned char *route;
    uint32_t id;
    uint64_t time;
    unsigned char nonce[_TRIP_NONCE];
    unsigned char pubkey[TRIP_KP_PUB];
    uint32_t credit;
    uint32_t max_stream;
    uint32_t max_message;
};

void
_trip_send_nq(_trip_router_t *r, _trip_connection_t *c);
_trip_connection_t *
_trip_send_dq(_trip_router_t *r);

void
_trip_set_state(_trip_router_t *r, enum _tripr_state state);

void
_trip_connection_notify(_trip_connection_t *);
void
_trip_connection_close(_trip_connection_t *);
bool
_trip_connection_send(_trip_connection_t *, _tripbuf_t *);
bool
_trip_connection_sendmore(_trip_connection_t *);
void
_trip_connection_handle_segment(_trip_connection_t *, int, int, unsigned char *);
void
_trip_connection_handle_open(_trip_connection_t *, _trip_open_t *);

int
trip_pack_len(char *format);
int
trip_pack(int cap, unsigned char *buf, char *format, ...);
int
trip_unpack(int blen, unsigned char *buf, char *format, ...);

#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_H_ */

