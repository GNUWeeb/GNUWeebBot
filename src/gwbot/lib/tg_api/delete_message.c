// SPDX-License-Identifier: GPL-2.0
/*
 *  src/gwbot/include/gwbot/lib/tg_api/delete_message.h
 *
 *  Send message API for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#include <gwbot/base.h>
#include <gwbot/lib/string.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/delete_message.h>


int tga_delete_msg(tga_handle_t *handle, const tga_delete_msg_t *ctx)
{
	char buf[128];

	snprintf(buf, sizeof(buf),
		 "chat_id=%" PRId64
		 "&message_id=%" PRIu64,
		 ctx->chat_id,
		 ctx->message_id);

	tga_set_body(handle, buf);
	tga_set_method(handle, "deleteMessage");
	return tg_api_post(handle);
}
