// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/modules/002_translate/header.h
 *
 *  Trnslate module for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */


#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <gwbot/module.h>
#include <gwbot/lib/string.h>

#include "header.h"

#define TEXT_BUFFER_SIZE (0x8000ull)
#define CURL_BUFFER_SIZE (TEXT_BUFFER_SIZE + 0x1000ull) /* JSON is longer */

GWMOD_NAME_DEFINE(002_translate, "Translate module");

int GWMOD_STARTUP_DEFINE(002_translate, struct gwbot_state *state)
{
	return 0;
}


int GWMOD_SHUTDOWN_DEFINE(002_translate, struct gwbot_state *state)
{
	return 0;
}


static int handle_translate_module(const char *payload, size_t payload_len,
				   struct tgev *evt,
				   const struct gwbot_thread *thread);


int GWMOD_ENTRY_DEFINE(002_translate, const struct gwbot_thread *thread,
				     struct tgev *evt)
{
	size_t text_len, rcx, payload_len;
	char payload[TEXT_BUFFER_SIZE + 0x30u], c, *payload_ptr = payload;
	const char *text = tge_get_text(evt), *back, *start_text;

	if (text == NULL)
		goto cancel;

	text_len = strnlen(text, TEXT_BUFFER_SIZE / 3u);
	if (text_len > ((TEXT_BUFFER_SIZE / 3u) - 1u))
		goto cancel;

	start_text = text;

	c = *text;
	if (c == '!' || c == '.' || c == '/' || c == '~')
		text++;

	c = *text++;
	if (c != 't')
		goto cancel;

	c = *text++;
	if (c != 'r' && c != 'l')
		goto cancel;

	c = *text++;
	if (c != ' ')
		goto cancel;


	/*
	 * Skip white space after command name.
	 */
	while (*text == ' ')
		text++;


	/*
	 * Take source language code (from)
	 */
	rcx = 0;
	back = text;
	while (*text != ' ') {
		text++;
		rcx++;
		if (rcx >= 5)
			goto cancel;
	}

	memcpy(payload_ptr, "fr=\0", 4);
	payload_ptr += 3;
	/*
	 * TODO: Make urlencode less dangerous
	 */
	urlencode(payload_ptr, back, rcx, true);
	while (*payload_ptr != '\0')
		payload_ptr++;


	/*
	 * Skip white space after source language
	 */
	while (*text == ' ')
		text++;


	/*
	 * Take target language code (to)
	 */
	rcx = 0;
	back = text;
	while (*text != ' ') {
		text++;
		rcx++;
		if (rcx >= 5)
			goto cancel;
	}

	memcpy(payload_ptr, "&to=", 4);
	payload_ptr += 4;
	/*
	 * TODO: Make urlencode less dangerous
	 */
	urlencode(payload_ptr, back, rcx, true);
	while (*payload_ptr != '\0')
		payload_ptr++;


	c = *text;
	if (c != ' ')
		goto cancel;


	/*
	 * Skip white space after target language
	 */
	while (*text == ' ')
		text++;

	memcpy(payload_ptr, "&text=\0\0", 8);
	payload_ptr += 6;
	/*
	 * TODO: Make urlencode less dangerous
	 */
	payload_len = text_len - (size_t)(text - start_text);
	urlencode(payload_ptr, text, payload_len, true);

	payload_len = (size_t)(payload_ptr - payload);
	while (*payload_ptr != '\0') {
		payload_ptr++;
		payload_len++;
	}

	return handle_translate_module(payload, payload_len, evt, thread);
cancel:
	return -ECANCELED;
}


struct tr_wr {
	size_t	len;
	char	*res_text;
};

static size_t translate_write_callback(void *data, size_t size, size_t nmemb,
				       void *userp)
{
	struct tr_wr *mem = userp;
	size_t add_len = size * nmemb, len = mem->len;


	if ((add_len + len) >= (CURL_BUFFER_SIZE - 1u))
		/* 
		 * Run out of space
		 */
		return 0;


	memcpy(mem->res_text + len, data, add_len);
	mem->len += add_len;

	return add_len;
}

static int fetch_api(struct tr_wr *mem, const char *payload, size_t payload_len)
{
	int ret = 0;
	CURL *curl;
	CURLcode cres;
	

	curl = curl_easy_init();
	if (unlikely(curl == NULL)) {
		pr_err("curl_easy_init(): " PRERF, PREAR(ENOMEM));
		return -ENOMEM;
	}

	curl_easy_setopt(curl, CURLOPT_URL, "https://api.gnuweeb.org/google_translate.php");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload_len);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, translate_write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, mem);

	cres = curl_easy_perform(curl);
	if (unlikely(cres != CURLE_OK)) {
		pr_err("curl_easy_perform(): %s", curl_easy_strerror(cres));
		ret = -EBADMSG;
	} else {
		mem->res_text[mem->len] = '\0';
	}

	curl_easy_cleanup(curl);
	return ret;
}


static int send_reply_text(const struct gwbot_thread *thread,
			   const char *reply_text, struct tgev *evt)
{
	int ret;
	tga_handle_t thandle;

	tga_screate(&thandle, thread->state->cfg->cred.token);
	ret = tga_send_msg(&thandle, &(const tga_send_msg_t){
		.chat_id          = tge_get_chat_id(evt),
		.reply_to_msg_id  = tge_get_msg_id(evt),
		.text             = reply_text,
		.parse_mode       = PARSE_MODE_OFF
	});
	tga_sdestroy(&thandle);

	if (ret) {
		pr_err("tga_send_msg() on send_debug_message(): " PRERF,
		       PREAR(-ret));
	}

	return 0;
}


static int handle_translate_module(const char *payload, size_t payload_len,
				   struct tgev *evt,
				   const struct gwbot_thread *thread)
{
	int ret;
	json_object *obj = NULL, *res;
	const char *reply_text;

	{
		/*
		 *
		 * It is a big buffer, let the compiler reuse by specifiying
		 * the scope if possible.
		 *
		 */
		char res_text[CURL_BUFFER_SIZE];
		struct tr_wr mem;

		mem.len = 0;
		mem.res_text = res_text;

		ret = fetch_api(&mem, payload, payload_len);
		if (ret)
			goto out;

		obj = json_tokener_parse(res_text);
		if (obj == NULL) {
			reply_text = "Error: Invalid JSON data from API";
			goto out_reply;
		}
	}


	if (json_object_object_get_ex(obj, "result", &res)) {
		const char *ret = json_object_get_string(res);
		reply_text = ret ? ret : "Error: `result` is NULL";
	} else
	if (json_object_object_get_ex(obj, "error", &res)) {
		const char *err  = json_object_get_string(res);
		reply_text = err ? err : "Error: unknown error!";
	}


out_reply:
	ret = send_reply_text(thread, reply_text, evt);

	if (obj)
		json_object_put(obj);
out:
	return ret;
}
