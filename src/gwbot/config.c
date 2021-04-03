// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/config.c
 *
 *  Config parser for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#include <string.h>
#include <stdlib.h>
#include <inih/inih.h>
#include <gwbot/common.h>
#include <gwbot/lib/arena.h>
#include <gwbot/lib/string.h>


struct parse_struct {
	bool  			exec;
	struct_pad(0, sizeof(struct srv_cfg *) - sizeof(bool));
	struct gwbot_cfg	*cfg;
};


static int parser_handler(void *user, const char *section, const char *name,
			  const char *value, int lineno)
{
	struct parse_struct *ctx = user;
	struct gwbot_cfg *cfg = ctx->cfg;


	/* Section match */
	#define rmatch_s(STR) if (unlikely(!strcmp(section, (STR))))
	/* Name match */
	#define rmatch_n(STR) if (unlikely(!strcmp(name, (STR))))


	rmatch_s("credential") {
		rmatch_n("token") {
			cfg->cred.token = ar_strndup(value, 255);
		} else {
			goto out_invalid_name;
		}
	} else
	rmatch_s("storage") {
		/* TODO: Parse storage config */
	} else
	rmatch_s("mysql") {
		rmatch_n("use_db") {
			cfg->use_db = (bool)atoi(value);
		} else
		rmatch_n("host") {
			cfg->mysql.host = ar_strndup(value, 255);
		} else
		rmatch_n("port") {
			cfg->mysql.port = (uint16_t)atoi(value);
		} else
		rmatch_n("user") {
			cfg->mysql.user = ar_strndup(value, 255);
		} else
		rmatch_n("pass") {
			cfg->mysql.pass = ar_strndup(value, 255);
		} else
		rmatch_n("dbname") {
			cfg->mysql.db = ar_strndup(value, 255);
		} else
		rmatch_n("unix_sock") {
			cfg->mysql.unix_sock = ar_strndup(value, 255);
		} else {
			goto out_invalid_name;
		}
	} else
	rmatch_s("socket") {
		rmatch_n("bind_addr") {
			cfg->sock.bind_addr = ar_strndup(value, 255);
		} else
		rmatch_n("bind_port") {
			cfg->sock.bind_port = (uint16_t)atoi(value);
		} else
		rmatch_n("backlog") {
			cfg->sock.backlog = atoi(value);
		} else {
			goto out_invalid_name;
		}
	} else
	rmatch_s("worker"){
		rmatch_n("thread_c") {
			cfg->worker.thread_c = (uint16_t)atoi(value);
		} else {
			goto out_invalid_name;
		}
	} else {
		pr_error("Invalid section \"%s\" on line %d", section, lineno);
		goto out_err;
	}



	return true;

	#undef rmatch_n
	#undef rmatch_s
out_invalid_name:
	pr_error("Invalid name \"%s\" in section \"%s\" on line %d", name,
		 section, lineno);
out_err:
	ctx->exec = false;
	return false;
}


int gwbot_cfg_parse(struct gwbot_cfg *cfg)
{
	int err;
	int retval = 0;
	FILE *fhandle = NULL;
	struct parse_struct ctx;
	char *cfg_file = cfg->cfg_file;

	ctx.exec = true;
	ctx.cfg = cfg;
	if (cfg_file == NULL || *cfg_file == '\0')
		return 0;

	fhandle = fopen(cfg_file, "r");
	if (fhandle == NULL) {
		err = errno;
		pr_err("Can't open config file: \"%s\" " PRERF, cfg_file,
		       PREAR(err));
		return -err;
	}

	if (ini_parse_file(fhandle, parser_handler, &ctx) < 0) {
		retval = -1;
		goto out;
	}

	if (!ctx.exec) {
		retval = -EINVAL;
		pr_err("Error loading config file \"%s\"!", cfg_file);
		goto out;
	}

out:
	if (fhandle)
		fclose(fhandle);

	return retval;
}
