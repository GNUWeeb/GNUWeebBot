// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/event_handler.c
 *
 *  GNUWeebBot event handler.
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#include <gwbot/gwbot.h>
#include <gwbot/lib/tg_event.h>
#include <gwbot/event_handler.h>
#include <gwbot/lib/tg_api/send_message.h>

#include <gwbot/modules/000_debug/header.h>
#include <gwbot/modules/001_standard/header.h>

static const struct module_table mod_tbl[] = {
	GWMOD_REG(000_debug, TGEV_TEXT | TGEV_PHOTO),
	GWMOD_REG_END
};


int gwbot_event_handler(const struct gwbot_thread *thread)
{
	int ret;
	struct tgev evt;
	const char *json_str = thread->uni_pkt.pkt.data;
	const size_t mod_tbl_len = sizeof(mod_tbl) / sizeof(*mod_tbl);


	ret = tg_event_load_str(json_str, &evt);
	if (unlikely(ret < 0)) {
		pr_err("tg_event_load_str(): " PRERF, PREAR(-ret));
		return ret;
	}


	for (size_t i = 0; i < mod_tbl_len; i++) {
		uint64_t event_mask = mod_tbl[i].event_mask;

		if (unlikely((event_mask & evt.type) == 0))
			continue;

		ret = mod_tbl[i].entry(thread);
		if (unlikely(ret != -ECANCELED))
			break;
	}


	prl_notice(5, "Finished!");
	tg_event_destroy(&evt);
	return ret;
}
