// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/modules/001_standard/standard.c
 *
 *  Trnslate module for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */


#include <stdio.h>
#include <unistd.h>
#include <json-c/json.h>
#include <gwbot/module.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/string.h>

#include "header.h"

GWMOD_NAME_DEFINE(001_standard, "Standard module");

int GWMOD_STARTUP_DEFINE(001_standard, struct gwbot_state *state)
{
	return 0;
}


int GWMOD_SHUTDOWN_DEFINE(001_standard, struct gwbot_state *state)
{
	return 0;
}

int GWMOD_ENTRY_DEFINE(001_standard, const struct gwbot_thread *thread,
				     struct tgev *evt)
{
	return -ECANCELED;
}
