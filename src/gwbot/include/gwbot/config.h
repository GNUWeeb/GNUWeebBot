// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/print.h
 *
 *  Base header for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__CONFIG_H
#define GWBOT__CONFIG_H

struct gwbot_cfg {
	char		*cfg_file;
	char		*token;
};


int gwbot_cfg_parse(struct gwbot_cfg *cfg);


#endif /* #ifndef GWBOT__CONFIG_H */
