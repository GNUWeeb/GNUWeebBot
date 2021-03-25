// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/string.h
 *
 *  String helpers header for TeaVPN2
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__STRING_H
#define GWBOT__LIB__STRING_H

#include <stddef.h>
#include <string.h>


char *escapeshellarg(char *alloc, const char *str, size_t len, size_t *res_len);
char *trim_len(char *head, size_t len, size_t *res_len);
char *trim_len_cpy(char *head, size_t len, size_t *res_len);
char *trim(char *str);
char *trim_cpy(char *str);
char *trunc_str(char *str, size_t n);
void *memzero_explicit(void *s, size_t n);
int memcmp_explicit(const void *s1, const void *s2, size_t n);
char *urlencode(char *alloc, const char *s, size_t len, bool raw);
size_t htmlspecialchars(char *input, char *output);

#if !defined(__clang__) && defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

inline static char *sane_strncpy(char * __restrict__ dest,
				 const char * __restrict__ src,
				 size_t n)
{
	dest = strncpy(dest, src, n - 1);
	dest[n - 1] = '\0';
	return dest;
}

#if !defined(__clang__) && defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

#endif /* #ifndef GWBOT__LIB__STRING_H */
