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


GWMOD_NAME_DEFINE(000_debug, "GNUWeebBot Debug Module (first created module)");

int GWMOD_STARTUP_DEFINE(000_debug, struct gwbot_state *state)
{
	return 0;
}


int GWMOD_SHUTDOWN_DEFINE(000_debug, struct gwbot_state *state)
{
	return 0;
}


int GWMOD_ENTRY_DEFINE(000_debug, const struct gwbot_thread *thread,
				  struct tgev *evt)
{
	size_t len, hlen;
	tg_api_handle *thandle;
	char reply_text[0x9000];
	struct json_object *json_obj;
	struct gwbot_cfg *cfg = thread->state->cfg;
	const char *text, *json_pretty, *json_str;

	text = tge_get_text(evt);
	if (text == NULL)
		return -ECANCELED;

	if (strncmp(text, "/debug", 6) != 0)
		return -ECANCELED;


	if (thread->ev_from_sqe)
		json_str = thread->sqe_node->data + sizeof(uint16_t);
	else
		json_str = thread->pkt.pkt.data;


	json_obj = json_tokener_parse(json_str);
	json_pretty = json_object_to_json_string_ext(
			json_obj,
			JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE
		);


	hlen = (sizeof(reply_text) - sizeof("<pre>") - sizeof("</pre>")) / 6;
	len  = htmlspecialchars(
		mempcpy(reply_text, "<pre>", 5),
		hlen,
		json_pretty,
		strnlen(json_pretty, hlen)
	);
	*(char *)mempcpy(&reply_text[5 + len - 1], "</pre>", 6) = '\0';


	thandle = tg_api_hcreate(cfg->cred.token);
	tg_api_send_msg(thandle, &(const struct tga_send_msg){
		.chat_id		= tge_get_chat_id(evt),
		.reply_to_msg_id	= tge_get_msg_id(evt),
		.text			= reply_text,
		.parse_mode		= PARSE_MODE_HTML
	});
	tg_api_destroy(thandle);
	json_object_put(json_obj);
	return 0;
}

/*
 *
 * WARNING: Once the entry returns, the `evt` won't be valid anymore
 *
 */
// int GWMOD_ENTRY_DEFINE(000_debug, const struct gwbot_thread *thread,
// 				  struct tgev *evt)
// {
	// size_t len, hlen;
	// tg_api_handle *thandle;
	// char reply_text[0x9000];
	// struct json_object *json_obj;
	// struct gwbot_cfg *cfg = thread->state->cfg;
	// const char *text, *json_pretty, *json_str = thread->uni_pkt.pkt.data;

	// text = tge_get_text(evt);
	// if (text == NULL)
	// 	return 0;

	// if (strncmp(text, "/debug", 6) != 0)
	// 	return 0;

	// json_obj = json_tokener_parse(json_str);
	// json_pretty = json_object_to_json_string_ext(
	// 		json_obj,
	// 		JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE
	// 	);


	// hlen = (sizeof(reply_text) - sizeof("<pre>") - sizeof("</pre>")) / 6;
	// len  = htmlspecialchars(
	// 	mempcpy(reply_text, "<pre>", 5),
	// 	hlen,
	// 	json_pretty,
	// 	strnlen(json_pretty, hlen)
	// );
	// *(char *)mempcpy(&reply_text[5 + len - 1], "</pre>", 6) = '\0';


	// thandle = tg_api_hcreate(cfg->cred.token);
	// tg_api_send_msg(thandle, &(const struct tga_send_msg){
	// 	.chat_id		= tge_get_chat_id(evt),
	// 	.reply_to_msg_id	= tge_get_msg_id(evt),
	// 	.text			= reply_text,
	// 	.parse_mode		= PARSE_MODE_HTML
	// });
	// tg_api_destroy(thandle);
	// json_object_put(json_obj);
	// return 0;
// }
