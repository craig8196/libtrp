
#include "libtrp.h"
#include "_libtrp.h"
#include <sodium.h>

typedef struct _trip_signature_s
{
    unsigned char pub[crypto_sign_PUBLICKEYBYTES];
    unsigned char sec[crypto_sign_SECRETKEYBYTES];
} _trip_signature_t;

typedef struct _trip_keypair_s
{
    unsigned char pub[crypto_box_PUBLICKEYBYTES];
    unsigned char sec[crypto_box_SECRETKEYBYTES];
} _trip_keypair_t;

typedef struct _trip_nonce_s
{
    unsigned char nonce[crypto_box_NONCEBYTES];
} _trip_nonce_t;

trip_signature_t *
trip_signature_new(trip_memory_t *m)
{
    _trip_signature_t *sig = m->alloc(m->ud, sizeof(_trip_signature_t));

    if (sig)
    {
        /* Technically the function cannot fail, however, we're cautious. */
        if (crypto_sign_keypair(sig->pub, sig->sec))
        {
            m->free(m->ud, sig);
            sig = NULL;
        }
    }

    return (trip_signature_t *)sig;
}

void
trip_signature_free(trip_memory_t *m, trip_signature_t *sig)
{
    m->free(m->ud, sig);
}

trip_keypair_t *
trip_keypair_new(trip_memory_t *m)
{
    _trip_keypair_t *pair = m->alloc(m->ud, sizeof(_trip_keypair_t));

    if (pair)
    {
        /* Technically the function cannot fail, however, we're cautious. */
        if (crypto_box_keypair(pair->pub, pair->sec))
        {
            m->free(m->ud, pair);
            pair = NULL;
        }
    }

    return (trip_keypair_t *)pair;
}

void
trip_keypair_free(trip_memory_t *m, trip_keypair_t *pair)
{
    m->free(m->ud, pair);
}

void
_trip_nonce_fill(_trip_nonce_t *n)
{
    randombytes_buf(n->nonce, sizeof(n->nonce));
}

