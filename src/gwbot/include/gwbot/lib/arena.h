// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/arena.h
 *
 *  Arena header for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__ARENA_H
#define GWBOT__LIB__ARENA_H

#include <stddef.h>


void ar_init(char *ar, size_t size);
size_t ar_unused_size(void);
void *ar_alloc(size_t len);
void *ar_strdup(const char *str);
void *ar_strndup(const char *str, size_t inlen);

#endif /* #ifndef GWBOT__LIB__ARENA_H */
