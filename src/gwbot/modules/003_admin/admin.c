// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/modules/003_admin/admin.c
 *
 *  Trnslate module for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */


#include <stdio.h>
#include <unistd.h>
#include <json-c/json.h>
#include <gwbot/module.h>
#include <gwbot/lib/string.h>

#include "header.h"

GWMOD_NAME_DEFINE(003_admin, "Admin module");

int GWMOD_STARTUP_DEFINE(003_admin, struct gwbot_state *state)
{
	return 0;
}


int GWMOD_SHUTDOWN_DEFINE(003_admin, struct gwbot_state *state)
{
	return 0;
}


static bool is_sudoer(uint64_t user_id)
{
	static uint64_t sudoers[] = {
		701123895,	// lappretard
		1213668471,	// nrudesu
		133862899,	// ryne4s,
		243692601,	// ammarfaizi2
	};

	for (size_t i = 0; i < (sizeof(sudoers)/sizeof(*sudoers)); i++) {
		if (sudoers[i] == user_id)
			return true;
	}

	return false;
}


int GWMOD_ENTRY_DEFINE(003_admin, const struct gwbot_thread *thread,
				     struct tgev *evt)
{
	char c;
	int ret = -ECANCELED;
	const char *text = tge_get_text(evt);

	if (text == NULL)
		goto out;

	c = *text;
	if ((c != '!') && (c != '/') && (c != '.') && (c != '~'))
		goto out;

	text++;

out:
	return ret;
}
