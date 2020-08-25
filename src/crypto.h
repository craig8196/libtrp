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
 * @file crypto.h
 * @author Craig Jacobson
 * @brief Lite wrapper for cryptography functions.
 */
#ifndef _LIBTRP_CRYPTO_H_
#define _LIBTRP_CRYPTO_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <sodium.h>


/* Shorten some cryptography calls. */
#define _TRIP_SIGN (crypto_sign_BYTES)
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


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_CRYPTO_H_ */

