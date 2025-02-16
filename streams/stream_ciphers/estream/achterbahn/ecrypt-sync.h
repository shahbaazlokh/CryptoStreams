/* ------------------------------------------------------------------------- *
 *
 *   Program:   A C H T E R B A H N  - 128 / 80
 *              Version 1.2 of the C reference implementation (optimized)
 *
 *   Authors:   Berndt M. Gammel, Email: gammel@matpack.de
 *              Rainer Goettfert, Email: rainer.goettfert@gmx.de
 *              Oliver Kniffler,  Email: oliver.kniffler@arcor.de
 *
 *  Language:   ANSI C99
 *
 *   Sources:   achterbahn.c
 *              achterbahn.h
 *              ecrypt-sync.h
 *
 *  Includes:   ecrypt-portable.h,
 *              ecrypt-config.h,
 *              ecrypt-machine.h
 *
 *  Makefile:   Makefile
 *
 * Platforms:   This program has been tested on the following platforms:
 *              gcc 3.4.4, Cygwin, Windows 2000
 *              gcc 4.1.0, S.u.S.E. Linux 10.1
 *
 * Copyright:   (C) 2005-2006 by Berndt M. Gammel, Rainer Goettfert,
 *                               and Oliver Kniffler
 *
 * ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- *
 * This is the ECRYPT eSTREAM API file "ecrypt-sync.h"
 * ------------------------------------------------------------------------- */

#ifndef ACHTERBAHN_SYNC
#define ACHTERBAHN_SYNC

#include "../../stream_interface.h"
#include "../ecrypt-portable.h"

namespace stream_ciphers {
namespace estream {

/* ------------------------------------------------------------------------- */

/* Cipher parameters */

/* ------------------------------------------------------------------------- *
 * The name of the cipher.
 * ------------------------------------------------------------------------- */

#define ACHTERBAHN_NAME "ACHTERBAHN-128/80"
#define ACHTERBAHN_PROFILE "_____"

#define ACHTERBAHN_AUTHORS "B. M. Gammel, R. Goettfert, O. Kniffler"

/* ------------------------------------------------------------------------- *
 * Specify which key and IV sizes are supported by your cipher. A user
 * should be able to enumerate the supported sizes by running the
 * following code:
 *
 * for (i = 0; ACHTERBAHN_KEYSIZE(i) <= ACHTERBAHN_MAXKEYSIZE; ++i)
 *   {
 *     keysize = ACHTERBAHN_KEYSIZE(i);
 *
 *     ...
 *   }
 *
 * All sizes are in bits.
 * ------------------------------------------------------------------------- */

#define ACHTERBAHN_MAXKEYSIZE 128U /* variable keysize: 40 to 128 bit */
#define ACHTERBAHN_KEYSIZE(i) (40U + i * 8)

#define ACHTERBAHN_MAXIVSIZE 128U /* variable IV size: 0,8,16,...,keysize */
#define ACHTERBAHN_IVSIZE(i) (i * 8)

/* ------------------------------------------------------------------------- */

/* Data structures */

/* ------------------------------------------------------------------------- *
 * ACHTERBAHN_ctx is the structure containing the representation of the
 * internal state of your cipher.
 * ------------------------------------------------------------------------- */

typedef struct {
    /* IV length in units of bytes */
    u32 ivsize8;

    /* set to one if key length > 80, zero otherwise */
    u32 longkey;

    /* states of NLFSRs */
    u32 A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11;
    u64 A12;

    /* saved states */
    u32 sA0, sA1, sA2, sA3, sA4, sA5, sA6, sA7, sA8, sA9, sA10, sA11;
    u64 sA12;

} ACHTERBAHN_ctx;

class ECRYPT_Achterbahn : public estream_interface {
    ACHTERBAHN_ctx _ctx;

public:
    /* Mandatory functions */

    /* ------------------------------------------------------------------------- *
     * Key and message independent initialization. This function will be
     * called once when the program starts (e.g., to build expanded S-box
     * tables).
     * ------------------------------------------------------------------------- */

    void ECRYPT_init() override;

    /* ------------------------------------------------------------------------- *
     * Key setup. It is the user's responsibility to select the values of
     * keysize and ivsize from the set of supported values specified above.
     * ------------------------------------------------------------------------- */

    void ECRYPT_keysetup(const u8* key,
                         u32 keysize,          /* Key size in bits. */
                         u32 ivsize) override; /* IV size in bits. */

    /* ------------------------------------------------------------------------- *
     * IV setup. After having called ECRYPT_keysetup(), the user is
     * allowed to call ECRYPT_ivsetup() different times in order to
     * encrypt/decrypt different messages with the same key but different IV's.
     * ------------------------------------------------------------------------- */

    void ECRYPT_ivsetup(const u8* iv) override;

    /* ------------------------------------------------------------------------- *
     * Encryption/decryption of arbitrary length messages.
     *
     * For efficiency reasons, the API provides two types of
     * encrypt/decrypt functions. The ECRYPT_encrypt_bytes() function
     * (declared here) encrypts byte strings of arbitrary length, while
     * the ECRYPT_encrypt_blocks() function (defined later) only accepts
     * lengths which are multiples of ECRYPT_BLOCKLENGTH.
     *
     * The user is allowed to make multiple calls to
     * ECRYPT_encrypt_blocks() to incrementally encrypt a long message,
     * but he is NOT allowed to make additional encryption calls once he
     * has called ECRYPT_encrypt_bytes() (unless he starts a new message
     * of course). For example, this sequence of calls is acceptable:
     *
     * ECRYPT_keysetup();
     *
     * ECRYPT_ivsetup();
     * ECRYPT_encrypt_blocks();
     * ECRYPT_encrypt_blocks();
     * ECRYPT_encrypt_bytes();
     *
     * ECRYPT_ivsetup();
     * ECRYPT_encrypt_blocks();
     * ECRYPT_encrypt_blocks();
     *
     * ECRYPT_ivsetup();
     * ECRYPT_encrypt_bytes();
     *
     * The following sequence is not:
     *
     * ECRYPT_keysetup();
     * ECRYPT_ivsetup();
     * ECRYPT_encrypt_blocks();
     * ECRYPT_encrypt_bytes();
     * ECRYPT_encrypt_blocks();
     * ------------------------------------------------------------------------- */

    void ECRYPT_encrypt_bytes(const u8* plaintext,
                              u8* ciphertext,
                              u32 msglen) override; /* Message length in bytes. */

    void ECRYPT_decrypt_bytes(const u8* ciphertext,
                              u8* plaintext,
                              u32 msglen) override; /* Message length in bytes. */

/* ------------------------------------------------------------------------- */

/* Optional features */

/* ------------------------------------------------------------------------- *
 * For testing purposes it can sometimes be useful to have a function
 * which immediately generates keystream without having to provide it
 * with a zero plaintext. If your cipher cannot provide this function
 * (e.g., because it is not strictly a synchronous cipher), please
 * reset the ACHTERBAHN_GENERATES_KEYSTREAM flag.
 * ------------------------------------------------------------------------- */

#define ACHTERBAHN_GENERATES_KEYSTREAM
#ifdef ACHTERBAHN_GENERATES_KEYSTREAM

    void ACHTERBAHN_keystream_bytes(ACHTERBAHN_ctx* ctx,
                                    u8* keystream,
                                    u32 length); /* Length of keystream in bytes. */

#endif

/* ------------------------------------------------------------------------- */

/* Optional optimizations */

/* ------------------------------------------------------------------------- *
 * By default, the functions in this section are implemented using
 * calls to functions declared above. However, you might want to
 * implement them differently for performance reasons.
 * ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- *
 * All-in-one encryption/decryption of (short) packets.
 *
 * The default definitions of these functions can be found in
 * "ecrypt-sync.c". If you want to implement them differently, please
 * undef the ACHTERBAHN_USES_DEFAULT_ALL_IN_ONE flag.
 * ------------------------------------------------------------------------- */

#define ACHTERBAHN_USES_DEFAULT_ALL_IN_ONE

    void ACHTERBAHN_encrypt_packet(
            ACHTERBAHN_ctx* ctx, const u8* iv, const u8* plaintext, u8* ciphertext, u32 msglen);

    void ACHTERBAHN_decrypt_packet(
            ACHTERBAHN_ctx* ctx, const u8* iv, const u8* ciphertext, u8* plaintext, u32 msglen);

/* ------------------------------------------------------------------------- *
 * Encryption/decryption of blocks.
 *
 * By default, these functions are defined as macros. If you want to
 * provide a different implementation, please undef the
 * ACHTERBAHN_USES_DEFAULT_BLOCK_MACROS flag and implement the functions
 * declared below.
 * ------------------------------------------------------------------------- */

#define ACHTERBAHN_BLOCKLENGTH 4

#define ACHTERBAHN_USES_DEFAULT_BLOCK_MACROS
#ifdef ACHTERBAHN_USES_DEFAULT_BLOCK_MACROS

#define ACHTERBAHN_encrypt_blocks(ctx, plaintext, ciphertext, blocks)                              \
    ECRYPT_encrypt_bytes(ctx, plaintext, ciphertext, (blocks)*ACHTERBAHN_BLOCKLENGTH)

#define ACHTERBAHN_decrypt_blocks(ctx, ciphertext, plaintext, blocks)                              \
    ECRYPT_decrypt_bytes(ctx, ciphertext, plaintext, (blocks)*ACHTERBAHN_BLOCKLENGTH)

#ifdef ACHTERBAHN_GENERATES_KEYSTREAM

#define ACHTERBAHN_keystream_blocks(ctx, keystream, blocks)                                        \
    ACHTERBAHN_AE_keystream_bytes(ctx, keystream, (blocks)*ACHTERBAHN_BLOCKLENGTH)

#endif

#else

    void ACHTERBAHN_encrypt_blocks(ACHTERBAHN_ctx* ctx,
                                   const u8* plaintext,
                                   u8* ciphertext,
                                   u32 blocks); /* Message length in blocks. */

    void ACHTERBAHN_decrypt_blocks(ACHTERBAHN_ctx* ctx,
                                   const u8* ciphertext,
                                   u8* plaintext,
                                   u32 blocks); /* Message length in blocks. */

#ifdef ACHTERBAHN_GENERATES_KEYSTREAM

    void ACHTERBAHN_keystream_blocks(ACHTERBAHN_AE_ctx* ctx,
                                     const u8* keystream,
                                     u32 blocks); /* Keystream length in blocks. */

#endif

#endif
};

/*
 * If your cipher can be implemented in different ways, you can use
 * the ACHTERBAHN_VARIANT parameter to allow the user to choose between
 * them at compile time (e.g., gcc -DACHTERBAHN_VARIANT=3 ...). Please
 * only use this possibility if you really think it could make a
 * significant difference and keep the number of variants
 * (ACHTERBAHN_MAXVARIANT) as small as possible (definitely not more than
 * 10). Note also that all variants should have exactly the same
 * external interface (i.e., the same ACHTERBAHN_BLOCKLENGTH, etc.).
 */
#define ACHTERBAHN_MAXVARIANT 1 /* [edit] */

#ifndef ACHTERBAHN_VARIANT
#define ACHTERBAHN_VARIANT 1
#endif

#if (ACHTERBAHN_VARIANT > ACHTERBAHN_MAXVARIANT)
#error this variant does not exist
#endif

/* ------------------------------------------------------------------------- */

} // namespace estream
} // namespace stream_ciphers

#endif
