// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/shell.h
 *
 *  Shell header for TeaVPN2
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__ARENA_H
#define GWBOT__LIB__ARENA_H

#include <stddef.h>


char *shell_exec(const char *cmd, char *buf, size_t buflen, size_t *outlen);

#endif /* #ifndef GWBOT__LIB__ARENA_H */
