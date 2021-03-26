// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/modules/000_debug/header.h
 *
 *  Debug module for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */


#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <json-c/json.h>
#include <gwbot/module.h>
#include <gwbot/lib/string.h>

#include "header.h"


static int64_t get_chat_id(struct tgev *evt)
{
	if (evt->type == TGEV_TEXT)
		return evt->msg_text.chat.id;
	if (evt->type == TGEV_PHOTO)
		return evt->msg_photo.chat.id;
	return -1;
}


static const char *get_text(struct tgev *evt)
{
	if (evt->type == TGEV_TEXT)
		return evt->msg_text.text;
	if (evt->type == TGEV_PHOTO)
		return evt->msg_photo.caption;
	return NULL;
}


static uint64_t get_msg_id(struct tgev *evt)
{
	if (evt->type == TGEV_TEXT)
		return evt->msg_text.msg_id;
	if (evt->type == TGEV_PHOTO)
		return evt->msg_photo.msg_id;
	return 0;
}



/*
 *
 * WARNING: Once the entry returns, the `evt` won't be valid anymore
 *
 */
int GWMOD_ENTRY_DEFINE(000_debug, const struct gwbot_thread *thread,
				  struct tgev *evt)
{
	size_t len;
	char reply_text[0x2500];
	tg_api_handle *thandle;
	struct json_object *json_obj;
	struct gwbot_cfg *cfg = thread->state->cfg;
	const char *text, *json_pretty, *json_str = thread->uni_pkt.pkt.data;


	text = get_text(evt);
	if (text == NULL)
		return 0;
	if (strncmp(text, "/debug", 6) != 0)
		return 0;


	json_obj = json_tokener_parse(json_str);
	json_pretty = json_object_to_json_string_ext(
			json_obj,
			JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE
		);


	len = htmlspecialchars(mempcpy(reply_text, "<pre>", 5), json_pretty);
	*(char *)mempcpy(&reply_text[5 + len], "</pre>", 6) = '\0';


	thandle = tg_api_hcreate(cfg->cred.token);
	tg_api_send_msg(thandle, &(const struct tga_send_msg){
		.chat_id		= get_chat_id(evt),
		.reply_to_msg_id	= get_msg_id(evt),
		.text			= reply_text,
		.parse_mode		= PARSE_MODE_HTML
	});
	tg_api_destroy(thandle);
	json_object_put(json_obj);
	return 0;
}
