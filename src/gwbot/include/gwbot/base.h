// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/base.h
 *
 *  Base header for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__BASE_H
#define GWBOT__BASE_H

#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdalign.h>
#include <inttypes.h>
#include <gwbot/print.h>
#include <gwbot/vt_hexdump.h>


#define TASSERT(EXPR) assert(EXPR)

#define likely(EXPR)   __builtin_expect(!!(EXPR), 1)
#define unlikely(EXPR) __builtin_expect(!!(EXPR), 0)

#ifndef static_assert
#  define static_assert(EXPR, ASSERT) _Static_assert((EXPR), ASSERT)
#endif

#ifndef offsetof
#  define offsetof(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
#endif

#ifndef member_size
#  define member_size(type, member) sizeof(((type *)0)->member)
#endif

#ifndef INET_ADDRSTRLEN
#  define IPV4_L (sizeof("xxx.xxx.xxx.xxx"))
#else
#  define IPV4_L (INET_ADDRSTRLEN)
#endif

#ifndef INET6_ADDRSTRLEN
#  define IPV6_L (sizeof("xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxx.xxx.xxx.xxx"))
#else
#  define IPV6_L (INET6_ADDRSTRLEN)
#endif

#define IPV4_SL (IPV4_L + 8) /* For safer size */
#define IPV6_SL (IPV6_L + 8) /* For safer size */

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif

#ifndef __maybe_unused
#  define __maybe_unused __attribute__((unused))
#endif

#ifndef __inline
#  define __inline inline
#endif

#ifndef __always_inline
#  define __always_inline __inline __attribute__((always_inline))
#endif

#ifndef __no_inline
#  define __no_inline __attribute__((noinline))
#endif

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#if __has_attribute(__fallthrough__)
#  define fallthrough __attribute__((__fallthrough__))
#else
#  define fallthrough do {} while (0)  /* fallthrough */
#endif

#define struct_pad(N, SIZE) uint8_t __pad__##N[SIZE]

#define STR(a) #a
#define XSTR(a) STR(a)

#define GWBOT_VERSION \
	XSTR(VERSION) \
	"." \
	XSTR(PATCHLEVEL) \
	"." \
	XSTR(SUBLEVEL) \
	EXTRAVERSION

static_assert(sizeof(char) == 1, "Bad sizeof(char)");
static_assert(sizeof(short) == 2, "Bad sizeof(short)");
static_assert(sizeof(int) == 4, "Bad sizeof(int)");
static_assert(sizeof(unsigned char) == 1, "Bad sizeof(unsigned char)");
static_assert(sizeof(unsigned short) == 2, "Bad sizeof(unsigned short)");
static_assert(sizeof(unsigned int) == 4, "Bad sizeof(unsigned int)");

static_assert(sizeof(bool) == 1, "Bad sizeof(bool)");

static_assert(sizeof(int8_t) == 1, "Bad sizeof(int8_t)");
static_assert(sizeof(uint8_t) == 1, "Bad sizeof(uint8_t)");

static_assert(sizeof(int16_t) == 2, "Bad sizeof(int16_t)");
static_assert(sizeof(uint16_t) == 2, "Bad sizeof(uint16_t)");

static_assert(sizeof(int32_t) == 4, "Bad sizeof(int32_t)");
static_assert(sizeof(uint32_t) == 4, "Bad sizeof(uint32_t)");

static_assert(sizeof(int64_t) == 8, "Bad sizeof(int64_t)");
static_assert(sizeof(uint64_t) == 8, "Bad sizeof(uint64_t)");

/* We only support 32-bit and 64-bit pointer size. */
static_assert((sizeof(void *) == 8) || (sizeof(void *) == 4),
	      "Bad sizeof(void *)");


#include <gwbot/config.h>

#endif /* #ifndef GWBOT__BASE_H */
