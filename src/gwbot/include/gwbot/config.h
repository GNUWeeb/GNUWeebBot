// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/config.h
 *
 *  Base header for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__CONFIG_H
#define GWBOT__CONFIG_H


struct gwbot_sock_cfg {
	char			*bind_addr;
	int			backlog;
	uint16_t		bind_port;
	struct_pad(0, 2);
};

struct gwbot_cred_cfg {
	char			*token;
};


struct gwbot_wrk_cfg {
	uint16_t		thread_c;
};


struct gwbot_mysql_cfg {
	char			*host;
	char			*user;
	char			*pass;
	char			*db;
	uint16_t		port;
	char			*unix_sock;
	unsigned int		flags;
};


struct gwbot_cfg {
	char			*cfg_file;
	struct gwbot_cred_cfg	cred;
	struct gwbot_sock_cfg	sock;
	struct gwbot_wrk_cfg    worker;
	struct gwbot_mysql_cfg	mysql;
	bool			use_db;
};


int gwbot_cfg_parse(struct gwbot_cfg *cfg);


#endif /* #ifndef GWBOT__CONFIG_H */
