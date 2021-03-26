// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/modules/000_debug/header.h
 *
 *  Debug module for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#include <stdio.h>
#include <json-c/json.h>
#include <gwbot/module.h>
#include <gwbot/lib/string.h>

#include "header.h"

int GWMOD_ENTRY_DEFINE(000_debug, const struct gwbot_thread *thread)
{
	size_t len;
	char buf[0x2500];
	const char *json_str = thread->uni_pkt.pkt.data;


	len = htmlspecialchars(buf, json_str);

	printf("jsn = %s\n", json_str);
	printf("buf = %s\n", buf);
	printf("len = %zu\n", len);

	return 0;
}
