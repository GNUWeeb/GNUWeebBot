// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/modules/002_translate/translate.c
 *
 *  Trnslate module for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */


#include <stdio.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <gwbot/module.h>
#include <gwbot/lib/string.h>

#include "header.h"

#define CODE_BUFFER_SIZE (0x20ull)
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

static int do_translate(const struct gwbot_thread *thread, struct tgev *evt);



int GWMOD_ENTRY_DEFINE(002_translate, const struct gwbot_thread *thread,
				     struct tgev *evt)
{
	printf("test x\n");
	return do_translate(thread, evt);
}


static inline char my_tolower(char c)
{
	return ('A' <= c && c <= 'Z') ? c + 32 : c;
}


static inline bool is_ws(char c)
{
	return (c == ' ') || (c == '\n') || (c == '\t') || (c == '\r');
}


static int parse_command(const char *text, char *payload, size_t *payload_len_p,
			 struct tgev **evt_p)
{
	char c;
	size_t rcx;
	size_t text_len;
	size_t rt_text_len;
	size_t payload_len;
	char *payload_ptr = payload;
	const char *back;
	const char *rt_text = NULL;
	const char *start_text = text;
	struct tgev *reply_to = tge_get_reply_to(*evt_p);

	text_len = strnlen(text, TEXT_BUFFER_SIZE / 4u);
	if (text_len > ((TEXT_BUFFER_SIZE / 4u) - 1u))
		/*
		 * Message is too long (probably impossible to happen),
		 * just for safety check.
		 */
		goto cancel;

	c = *text;
	if (c == '!' || c == '.' || c == '/' || c == '~')
		text++;

	c = my_tolower(*text++);
	if (c != 't')
		goto cancel;

	c = my_tolower(*text++);
	if (c != 'r' && c != 'l')
		goto cancel;

	if (reply_to)
		rt_text = tge_get_text(reply_to);

	c = *text++;
	if (!is_ws(c)) {
		if (c == '\0')
			/*
			 * It is translate command with
			 * no argument like: `tr`
			 */
			goto out_replied_0_arg;

		goto cancel;
	}

	/*
	 * Skip white space after command name.
	 */
	while (is_ws(*text))
		text++;

	if (*text == '\0')
		/*
		 * It is translate command with
		 * no argument like: `tr   ` (with trailing whitespace)
		 */
		goto out_replied_0_arg;


	/*
	 * Take source language code (`from`)
	 */
	rcx  = 0;
	back = text;
	while ((!is_ws(*text)) && (*text != '\0')) {
		rcx++;
		text++;
		if (rcx >= CODE_BUFFER_SIZE)
			goto cancel;
	}

	memcpy(payload_ptr, "fr=\0", 4);
	payload_ptr += 3;
	/*
	 * TODO: Make urlencode less dangerous
	 */
	urlencode(payload_ptr, back, rcx, true);
	payload_ptr += strlen(payload_ptr);

	/*
	 * Skip white space after source language.
	 */
	while (is_ws(*text))
		text++;


	if (*text == '\0') {
		/*
		 * Potential reply to message translate.
		 * Message: `tr {to}`
		 *
		 * With replied message.
		 */

		/*
		 * Update `fr` to `to`.
		 */
		memcpy(payload, "to", 2);
		goto out_replied_1_arg;
	}


	/*
	 *
	 * Take target language code (`to`)
	 *
	 */
	rcx  = 0;
	back = text;
	while ((!is_ws(*text)) && (*text != '\0')) {
		rcx++;
		text++;
		if (rcx >= CODE_BUFFER_SIZE)
			goto cancel;
	}
	memcpy(payload_ptr, "&to=", 4);
	payload_ptr += 4;
	/*
	 * TODO: Make urlencode less dangerous
	 */
	urlencode(payload_ptr, back, rcx, true);
	payload_ptr += strlen(payload_ptr);

	/*
	 * Skip white space after target language.
	 */
	while (is_ws(*text))
		text++;

	if (*text == '\0')
		/*
		 * It is translate command with 2 arguments:
		 * `tr {from} {to}`
		 *
		 * It has potential to be translate replied message.
		 */
		goto out_replied_2_arg;


	memcpy(payload_ptr, "&text=\0\0", 8);
	payload_ptr += 6;
	/*
	 * TODO: Make urlencode less dangerous
	 */
	payload_len = text_len - (size_t)(uintptr_t)(text - start_text);
	urlencode(payload_ptr, text, payload_len, true);
	payload_ptr += strlen(payload_ptr);
	payload_len  = (size_t)(uintptr_t)(payload_ptr - payload);

	*payload_len_p = payload_len;
	return 0;

out_replied_0_arg:
	memcpy(payload_ptr, "to=en\0\0\0", 8);
	payload_ptr += 5;
out_replied_1_arg:
	memcpy(payload_ptr, "&fr=auto", 8);
	payload_ptr += 8;
out_replied_2_arg:
	if (rt_text == NULL)
		goto cancel;

	rt_text_len = strnlen(rt_text, TEXT_BUFFER_SIZE / 4u);
	if (rt_text_len > ((TEXT_BUFFER_SIZE / 4u) - 1u))
		/*
		 * Message is too long (probably impossible to happen),
		 * just for safety.
		 */
		goto cancel;

	memcpy(payload_ptr, "&text=\0\0", 8);
	payload_ptr += 6;

	/*
	 * TODO: Make urlencode less dangerous
	 */
	urlencode(payload_ptr, rt_text, rt_text_len, true);

	payload_len  = (size_t)(uintptr_t)(payload_ptr - payload);
	payload_len += strlen(payload_ptr);

	*payload_len_p = payload_len;
	*evt_p = reply_to;
	return 0;
cancel:
	return -ECANCELED;
}


static int do_translate(const struct gwbot_thread *thread, struct tgev *evt)
{
	int ret;
	size_t payload_len;
	char payload[TEXT_BUFFER_SIZE + (CODE_BUFFER_SIZE * 2)];
	const char *text = tge_get_text(evt);

	if (text == NULL) {
		ret = -ECANCELED;
		goto out;
	}

	ret = parse_command(text, payload, &payload_len, &evt);
	if (ret)
		goto out;

	return handle_translate_module(payload, payload_len, evt, thread);

out:
	return ret;
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

	printf("test\n");
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
		pr_err("tga_send_msg() on send_reply_text(): " PRERF,
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
		const char *ret_s = json_object_get_string(res);
		reply_text = ret_s ? ret_s : "Error: `result` is NULL";
	} else
	if (json_object_object_get_ex(obj, "error", &res)) {
		const char *err  = json_object_get_string(res);
		reply_text = err ? err : "Error: unknown error!";
	} else {
		reply_text = "Error: unknown error!";
	}


out_reply:
	ret = send_reply_text(thread, reply_text, evt);

	if (obj)
		json_object_put(obj);
out:
	return ret;
}
