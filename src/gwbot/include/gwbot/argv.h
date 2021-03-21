// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/print.h
 *
 *  Hexdump macro for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__ARGV_H
#define GWBOT__ARGV_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <gwbot/config.h>
#include <gwbot/base.h>

int gwbot_argv_parse(int argc, char *argv[], struct gwbot_cfg *cfg);

#endif /* #ifndef GWBOT__ARGV_H */
