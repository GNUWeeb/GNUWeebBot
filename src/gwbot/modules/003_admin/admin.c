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
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/string.h>

#include "header.h"

GWMOD_NAME_DEFINE(003_admin, "Admin module");


#define RTB_SIZE (0x400)

typedef enum _mod_cmd_t {
	CMD_NOP		= 0u,
	ADM_CMD_BAN	= (1u << 0u),
	ADM_CMD_UNBAN	= (1u << 1u),
	ADM_CMD_KICK	= (1u << 2u),
	ADM_CMD_WARN	= (1u << 3u),
	ADM_CMD_MUTE	= (1u << 4u),
	ADM_CMD_TMUTE	= (1u << 5u),
	ADM_CMD_UNMUTE	= (1u << 6u),
	ADM_CMD_PIN	= (1u << 7u),
	USR_CMD_REPORT	= (1u << 8u),
	USR_CMD_DELVOTE	= (1u << 9u),
} mod_cmd_t;

#define ADMIN_BITS			\
	(				\
		ADM_CMD_BAN	|	\
		ADM_CMD_UNBAN	|	\
		ADM_CMD_KICK	|	\
		ADM_CMD_WARN	|	\
		ADM_CMD_MUTE 	|	\
		ADM_CMD_TMUTE 	|	\
		ADM_CMD_UNMUTE	|	\
		ADM_CMD_PIN		\
	)



int GWMOD_STARTUP_DEFINE(003_admin, struct gwbot_state *state)
{
	return 0;
}


int GWMOD_SHUTDOWN_DEFINE(003_admin, struct gwbot_state *state)
{
	return 0;
}


static inline bool is_ws(char c)
{
	return (c == ' ') || (c == '\n') || (c == '\t') || (c == '\r');
}


static bool check_is_sudoer(uint64_t user_id)
{
	/*
	 * TODO: Use binary search for large sudoers list.
	 */
	static const uint64_t sudoers[] = {
		133862899ull,	// ryne4s,
		243692601ull,	// ammarfaizi2
		701123895ull,	// lappretard
		1213668471ull,	// nrudesu
		1472415329ull,	// mysticial
	};

	for (size_t i = 0; i < (sizeof(sudoers) / sizeof(*sudoers)); i++) {
		if (sudoers[i] == user_id)
			return true;
	}

	return false;
}


static bool process_json_msg(tga_handle_t *thandle, char *reply_text)
{
	bool ok = true;
	const char *json_res = tge_get_res_body(thandle);
	json_object *res;
	json_object *json_obj;

	json_obj = json_tokener_parse(json_res);
	if (json_obj == NULL) {
		ok = false;
		snprintf(reply_text, RTB_SIZE,
			 "Error: Cannot parse JSON response from API");
		goto out;
	}

	if (!json_object_object_get_ex(json_obj, "ok", &res) || !res) {
		ok = false;
		snprintf(reply_text, RTB_SIZE,
			 "Cannot find \"ok\" key from JSON API");
		goto out;
	}


	if (json_object_get_boolean(res))
		/*
		 * Successful!
		 */
		goto out;


	ok = false;
	if (!json_object_object_get_ex(json_obj, "description", &res)) {
		memcpy(reply_text, "Error: Cannot parse JSON response from API",
			43);
	} else {
		snprintf(reply_text, RTB_SIZE,
			 "Error: %s", json_object_get_string(res));
	}

out:
	json_object_put(json_obj);
	return ok;
}


static bool do_kick(const struct gwbot_thread *thread, char *reply_text,
		    const tga_kick_cm_t *arg)
{
	int ret;
	bool ok = true;
	tga_handle_t thandle;

	tga_screate(&thandle, thread->state->cfg->cred.token);
	ret = tga_kick_chat_member(&thandle, arg);


	if (ret) {
		pr_err("tga_kick_chat_member() on do_kick(): " PRERF,
		       PREAR(-ret));
		snprintf(reply_text, RTB_SIZE,
			 "Error: tga_kick_chat_member(): " PRERF, PREAR(-ret));
	} else {
		ok = process_json_msg(&thandle, reply_text);
	}

	tga_sdestroy(&thandle);
	return ok;
}


static bool do_unban(const struct gwbot_thread *thread, char *reply_text,
		     const tga_unban_cm_t *arg)
{
	int ret;
	bool ok = true;
	tga_handle_t thandle;

	tga_screate(&thandle, thread->state->cfg->cred.token);
	ret = tga_unban_chat_member(&thandle, arg);

	if (ret) {
		pr_err("tga_unban_chat_member() on do_unban(): " PRERF,
		       PREAR(-ret));
		snprintf(reply_text, RTB_SIZE,
			 "Error: tga_unban_chat_member(): " PRERF, PREAR(-ret));
	} else {
		ok = process_json_msg(&thandle, reply_text);
	}

	tga_sdestroy(&thandle);
	return ok;
}


static bool do_mute(const struct gwbot_thread *thread, char *reply_text,
		    const tga_restrict_cm_t *arg)
{
	int ret;
	bool ok = true;
	tga_handle_t thandle;

	tga_screate(&thandle, thread->state->cfg->cred.token);
	ret = tga_restrict_chat_member(&thandle, arg);


	if (ret) {
		pr_err("tga_restrict_chat_member() on do_mute(): " PRERF,
		       PREAR(-ret));
		snprintf(reply_text, RTB_SIZE,
			 "Error: tga_restrict_chat_member(): " PRERF,
			 PREAR(-ret));
	} else {
		ok = process_json_msg(&thandle, reply_text);
	}

	tga_sdestroy(&thandle);
	return ok;
}


static size_t gen_name_link(char *buf, size_t sps, const struct tgevi_from *fr)
{
	size_t ret = 0, tmp;

	if (sps < 32)
		return 0;

	/*
	 * Reserve space for </a> and null char.
	 */
	sps -= 5;

	tmp = (size_t)snprintf(buf, sps,
			       "<a href=\"tg://user?id=%" PRIu64 "\">", fr->id);

	ret += tmp;
	sps -= tmp;

	tmp = htmlspecialchars(buf + ret, sps, fr->first_name,
			       strnlen(fr->first_name, 0xfful));

	ret += tmp - 1;
	sps += tmp;

	if (fr->last_name) {
		buf[ret++] = ' ';
		tmp = htmlspecialchars(buf + ret, sps, fr->last_name,
				       strnlen(fr->last_name, 0xfful));
		ret += tmp - 1;
		sps += tmp;
	}


	memcpy(buf + ret, "</a>", 5);
	ret += 4;
	return ret;
}


static int send_reply(const struct gwbot_thread *thread, struct tgev *evt,
		      const char *reply_text, uint64_t msg_id)
{
	int ret;
	tga_handle_t thandle;

	tga_screate(&thandle, thread->state->cfg->cred.token);
	ret = tga_send_msg(&thandle, &(const tga_send_msg_t){
		.chat_id          = tge_get_chat_id(evt),
		.reply_to_msg_id  = msg_id,
		.text             = reply_text,
		.parse_mode       = PARSE_MODE_HTML
	});

	if (ret)
 		pr_err("tga_send_msg() on send_reply(): " PRERF, PREAR(-ret));

	tga_sdestroy(&thandle);

	return 0;
}


static int send_eperm(const struct gwbot_thread *thread, struct tgev *evt)
{
	static const char reply_text[] 
		= "You don't have permission to execute this command!";

	return send_reply(thread, evt, reply_text, tge_get_msg_id(evt));
}


static int exec_adm_cmd_ban(const struct gwbot_thread *thread, struct tgev *evt,
			    uint64_t target_uid, const char *reason)
{
	bool tmp;
	char reply_text[RTB_SIZE];
	uint64_t reply_to_msg_id;
	struct tgev *reply_to;

	tmp = do_kick(thread, reply_text, &(const tga_kick_cm_t){
		.chat_id = tge_get_chat_id(evt),
		.user_id = target_uid,
		.until_date = 0,
		.revoke_msg = false
	});

	reply_to_msg_id = tge_get_msg_id(evt);

	if (!tmp)
		goto out;


	reply_to = tge_get_reply_to(evt);
	if (reply_to) {
		size_t pos = 0, space = sizeof(reply_text);
		const struct tgevi_from *fr = tge_get_from(reply_to);

		pos = gen_name_link(reply_text, space, fr);
		space -= pos;
		memcpy(reply_text + pos, " has been banned!", 18);
		pos += 17;
		space -= 17;

		if (reason)
			snprintf(reply_text + pos, space, "\n<b>Reason:</b> %s",
				 reason);
	}

out:
	
	return send_reply(thread, evt, reply_text, reply_to_msg_id);
}


static int kick_or_unban(const struct gwbot_thread *thread,
			 struct tgev *evt, uint64_t target_uid,
			 const char *reason, bool is_unban)
{
	bool tmp;
	char reply_text[RTB_SIZE];
	uint64_t reply_to_msg_id;
	struct tgev *reply_to;

	tmp = do_unban(thread, reply_text, &(const tga_unban_cm_t){
		.chat_id = tge_get_chat_id(evt),
		.user_id = target_uid,
		.only_if_banned = is_unban
	});

	reply_to_msg_id = tge_get_msg_id(evt);

	if (!tmp)
		goto out;


	reply_to = tge_get_reply_to(evt);
	if (reply_to) {
		size_t pos = 0, space = sizeof(reply_text);
		const struct tgevi_from *fr = tge_get_from(reply_to);

		pos = gen_name_link(reply_text, space, fr);
		space -= pos;

		if (is_unban) {
			memcpy(reply_text + pos, " has been unbanned!", 20);
			pos += 19;
			space -= 19;
		} else {
			memcpy(reply_text + pos, " has been kicked!", 18);
			pos += 17;
			space -= 17;
		}

		if (reason)
			snprintf(reply_text + pos, space, "\n<b>Reason:</b> %s",
				 reason);
	}

out:
	return send_reply(thread, evt, reply_text, reply_to_msg_id);
}


static int exec_adm_cmd_unban(const struct gwbot_thread *thread,
			      struct tgev *evt, uint64_t target_uid,
			      const char *reason)
{
	return kick_or_unban(thread, evt, target_uid, reason, true);
}


static int exec_adm_cmd_kick(const struct gwbot_thread *thread,
			     struct tgev *evt, uint64_t target_uid,
			     const char *reason)
{
	return kick_or_unban(thread, evt, target_uid, reason, false);
}


static int mute_or_unmute(const struct gwbot_thread *thread, struct tgev *evt,
			  uint64_t target_uid, const char *reason,
			  bool is_unmute)
{
	bool tmp;
	char reply_text[RTB_SIZE];
	uint64_t reply_to_msg_id;
	struct tgev *reply_to;

	tmp = do_mute(thread, reply_text, &(const tga_restrict_cm_t){
		.chat_id = tge_get_chat_id(evt),
		.user_id = target_uid,
		.until_date = 0,
		.permissions = {
			.can_send_messages = is_unmute,
			.can_send_media_messages = is_unmute,
			.can_send_polls = is_unmute,
			.can_send_other_messages = is_unmute,
			.can_add_web_page_previews = is_unmute,
			.can_change_info = is_unmute,
			.can_invite_users = is_unmute,
			.can_pin_messages = is_unmute
		}
	});

	reply_to_msg_id = tge_get_msg_id(evt);

	if (!tmp)
		goto out;


	reply_to = tge_get_reply_to(evt);
	if (reply_to) {
		size_t pos = 0, space = sizeof(reply_text);
		const struct tgevi_from *fr = tge_get_from(reply_to);

		pos = gen_name_link(reply_text, space, fr);
		space -= pos;

		if (is_unmute) {
			memcpy(reply_text + pos, " has been unmuted!", 18);
			pos += 18;
			space -= 18;
		} else {
			memcpy(reply_text + pos, " has been muted!", 16);
			pos += 16;
			space -= 16;
		}
		

		if (reason)
			snprintf(reply_text + pos, space, "\n<b>Reason:</b> %s",
				 reason);
	}

out:
	
	return send_reply(thread, evt, reply_text, reply_to_msg_id);
}


static int exec_adm_cmd_mute(const struct gwbot_thread *thread, struct tgev *evt,
			     uint64_t target_uid, const char *reason)
{
	return mute_or_unmute(thread, evt, target_uid, reason, false);
}


static int exec_adm_cmd_unmute(const struct gwbot_thread *thread,
			       struct tgev *evt,
			       uint64_t target_uid, const char *reason)
{
	return mute_or_unmute(thread, evt, target_uid, reason, true);
}



int GWMOD_ENTRY_DEFINE(003_admin, const struct gwbot_thread *thread,
				     struct tgev *evt)
{
	size_t rcx;
	char c, yx[10];
	mod_cmd_t cmd = CMD_NOP;
	int ret = -ECANCELED;
	struct tgev *reply_to;
	uint64_t user_id, target_uid;
	const char *tx = tge_get_text(evt), *reason = NULL;

	if (tx == NULL)
		goto out;

	c = *tx++;
	if ((c != '!') && (c != '/') && (c != '.') && (c != '~'))
		goto out;


	rcx = 0;
	while (tx[rcx]) {
		c = tx[rcx];
		yx[rcx] = ('A' <= c && c <= 'Z') ? c + 32 : c;
		rcx++;

		if (rcx >= (sizeof(yx) - 1))
			break;
	}

	yx[rcx] = '\0';

	c =
	(!strncmp("ban",     yx, 3) && (tx += 3) && (cmd = ADM_CMD_BAN))     ||
	(!strncmp("unban",   yx, 5) && (tx += 5) && (cmd = ADM_CMD_UNBAN))   ||
	(!strncmp("kick",    yx, 4) && (tx += 4) && (cmd = ADM_CMD_KICK))    ||
	(!strncmp("warn",    yx, 4) && (tx += 4) && (cmd = ADM_CMD_WARN))    ||
	(!strncmp("mute",    yx, 4) && (tx += 4) && (cmd = ADM_CMD_MUTE))    ||
	(!strncmp("tmute",   yx, 5) && (tx += 5) && (cmd = ADM_CMD_TMUTE))   ||
	(!strncmp("unmute",  yx, 6) && (tx += 6) && (cmd = ADM_CMD_UNMUTE))  ||
	(!strncmp("pin",     yx, 3) && (tx += 3) && (cmd = ADM_CMD_PIN))     ||
	(!strncmp("report",  yx, 6) && (tx += 6) && (cmd = USR_CMD_REPORT))  ||
	(!strncmp("delvote", yx, 7) && (tx += 7) && (cmd = USR_CMD_DELVOTE));


	if (!c)
		goto out;


	reply_to = tge_get_reply_to(evt);
	user_id  = tge_get_user_id(evt);


	c = *tx;
	if (c == '\0') {

		if (!reply_to)
			/*
			 * The command requires a replied
			 * message or additional argument,
			 * but was not given.
			 */
			goto out;


		goto run_module;
	}


	if (!is_ws(c))
		goto out;


	/*
	 * Skip white space.
	 */
	while (is_ws(*tx))
		tx++;


	reason = tx;


run_module:
	if (reply_to == NULL) {
		if (reason == NULL) {
			
			// target_uid = ???
			goto out;
		}


		/*
		 * TODO: Parse the reason, it may contain
		 *       username, user_id, etc.
		 */
		goto out;
	} else {
		target_uid = tge_get_user_id(reply_to);
	}



	if (cmd & ADMIN_BITS) {
		/*
		 * This command requires administrator
		 * privilege.
		 */
		if (!check_is_sudoer(user_id)) {
			/*
			 * TODO: Send permission denied
			 * TODO: Group admin check
			 */
			ret = send_eperm(thread, evt);
			goto out;
		}
	}


	switch (cmd) {
	case CMD_NOP:
		abort();
	case ADM_CMD_BAN:
		ret = exec_adm_cmd_ban(thread, evt, target_uid, reason);
		break;
	case ADM_CMD_UNBAN:
		ret = exec_adm_cmd_unban(thread, evt, target_uid, reason);
		break;
	case ADM_CMD_KICK:
		ret = exec_adm_cmd_kick(thread, evt, target_uid, reason);
		break;
	case ADM_CMD_WARN:
		break;
	case ADM_CMD_MUTE:
		ret = exec_adm_cmd_mute(thread, evt, target_uid, reason);
		break;
	case ADM_CMD_TMUTE:
		break;
	case ADM_CMD_UNMUTE:
		ret = exec_adm_cmd_unmute(thread, evt, target_uid, reason);
		break;
	case ADM_CMD_PIN:
		break;
	case USR_CMD_REPORT:
		break;
	case USR_CMD_DELVOTE:
		break;
	}

out:
	return ret;
}
