// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/gwbot.h
 *
 *  Hexdump macro for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__GWBOT_H
#define GWBOT__GWBOT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <gwbot/base.h>
#include <gwbot/config.h>

int gwbot_run(struct gwbot_cfg *cfg);

#endif /* #ifndef GWBOT__GWBOT_H */
