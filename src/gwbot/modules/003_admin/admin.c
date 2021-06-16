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


int GWMOD_STARTUP_DEFINE(003_admin, struct gwbot_state *state)
{
	return 0;
}


int GWMOD_SHUTDOWN_DEFINE(003_admin, struct gwbot_state *state)
{
	return 0;
}


#define RTB_SIZE (0x400)


typedef enum _mod_cmd_t {
	CMD_NOP		= 0u,
	ADM_CMD_BAN	= (1u << 0u),
	ADM_CMD_SBAN	= (1u << 1u),
	ADM_CMD_UNBAN	= (1u << 2u),
	ADM_CMD_KICK	= (1u << 3u),
	ADM_CMD_SKICK	= (1u << 4u),
	ADM_CMD_WARN	= (1u << 5u),
	ADM_CMD_MUTE	= (1u << 6u),
	ADM_CMD_TMUTE	= (1u << 7u),
	ADM_CMD_UNMUTE	= (1u << 8u),
	ADM_CMD_PIN	= (1u << 9u),
	ADM_CMD_UNPIN	= (1u << 10u),
	ADM_CMD_DELETE	= (1u << 11u),
	USR_CMD_REPORT	= (1u << 12u),
	USR_CMD_DELVOTE	= (1u << 13u),
} mod_cmd_t;


/*
 * Commands that require admin privilege
 */
#define ADMIN_CMD_BITS			\
	(				\
		ADM_CMD_BAN	|	\
		ADM_CMD_SBAN	|	\
		ADM_CMD_UNBAN	|	\
		ADM_CMD_KICK	|	\
		ADM_CMD_SKICK	|	\
		ADM_CMD_WARN	|	\
		ADM_CMD_MUTE 	|	\
		ADM_CMD_TMUTE 	|	\
		ADM_CMD_UNMUTE	|	\
		ADM_CMD_PIN	|	\
		ADM_CMD_UNPIN	|	\
		ADM_CMD_DELETE		\
	)



static bool check_is_sudoer(uint64_t user_id)
{
	/*
	 * TODO: Use binary search for large sudoers list.
	 */
	static const uint64_t sudoers[] = {
		133862899ull,		// ryne4s,
		243692601ull,		// ammarfaizi2
		701123895ull,		// lappretard
		1213668471ull,		// nrudesu
		// 1472415329ull,	// mysticial
	};

	for (size_t i = 0; i < (sizeof(sudoers) / sizeof(*sudoers)); i++) {
		if (sudoers[i] == user_id)
			return true;
	}

	return false;
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
		= "Error: Operation not permitted (errno=EPERM) (code=1)";

	return send_reply(thread, evt, reply_text, tge_get_msg_id(evt));
}


static size_t gen_name_link(char *buf, size_t sps, uint64_t id,
			    const char *fisrt, const char *last)
{
	size_t ret = 0, tmp;

	if (sps < 32)
		return 0;

	/*
	 * Reserve space for </a> and null char.
	 */
	sps -= 5;
	tmp = (size_t)snprintf(buf, sps,
			       "<a href=\"tg://user?id=%" PRIu64 "\">", id);

	ret += tmp;
	sps -= tmp;

	tmp = htmlspecialchars(buf + ret, sps, fisrt, strnlen(fisrt, 0xfful));

	ret += tmp - 1;
	sps += tmp;

	if (last) {
		buf[ret++] = ' ';
		tmp = htmlspecialchars(buf + ret, sps, last,
				       strnlen(last, 0xfful));
		ret += tmp - 1;
		sps += tmp;
	}


	memcpy(buf + ret, "</a>", 5);
	ret += 4;
	return ret;
}


static int lookup_tg_privilege(const struct gwbot_thread *thread,
			       struct tgev *evt, uint64_t user_id,
			       int64_t chat_id)
{
	int ret;
	size_t admin_c = 0;
	tga_handle_t thandle;
	char reply_text[128];
	json_object *json_obj = NULL;
	const char *json_plain = NULL;
	struct tga_chat_member *admins = NULL;

	tga_screate(&thandle, thread->state->cfg->cred.token);
	ret = tga_get_chat_admins(&thandle, chat_id);
	if (ret) {
		snprintf(reply_text, 128, "Error: tga_get_chat_admins(): "
			 PRERF, PREAR(-ret));
		goto out;
	}


	json_plain = tge_get_res_body(&thandle);
	if (!json_plain) {
		ret = -EINVAL;
		snprintf(reply_text, 128, "Error: tge_get_res_body(): "
			 PRERF, PREAR(EINVAL));
		goto out;
	}


	json_obj = json_tokener_parse(json_plain);
	if (!json_obj) {
		ret = -EINVAL;
		snprintf(reply_text, 128, "Error: json_tokener_parse(): "
			 PRERF, PREAR(EINVAL));
		goto out;
	}


	ret = parse_tga_admins(json_obj, &admins, &admin_c);
	if (ret) {
		snprintf(reply_text, 128, "Error: parse_tga_admins(): "
			 PRERF, PREAR(-ret));
		goto out;
	}


	ret = -EPERM;
	for (size_t i = 0; i < admin_c; i++) {
		if (admins[i].user.id == user_id) {
			ret = 0;
			break;
		}
	}

out:
	if (json_obj)
		json_object_put(json_obj);

	free(admins);
	tga_sdestroy(&thandle);

	if (ret && ret != -EPERM)
		send_reply(thread, evt, reply_text, tge_get_msg_id(evt));

	return ret;
}


static inline bool is_privileged_user(const struct gwbot_thread *thread,
				      struct tgev *evt, uint64_t user_id)
{
	int ret;

	if (check_is_sudoer(user_id))
		return true;

	ret = lookup_tg_privilege(thread, evt, user_id, tge_get_chat_id(evt));
	if (!ret)
		return true;

	if (ret == -EPERM)
		send_eperm(thread, evt);
	else
		pr_err("lookup_privilege(): " PRERF, PREAR(-ret));

	return false;
}


static inline bool is_ws(char c)
{
	return (c == ' ') || (c == '\n') || (c == '\t') || (c == '\r');
}


static bool process_json_msg(tga_handle_t *thandle, char *reply_text)
{
	bool ok = true;
	json_object *res = NULL, *json_obj = NULL;
	const char *json_plain = tge_get_res_body(thandle);
	static const char err_parse[] =
		"Error: Cannot parse JSON response from API";


	json_obj = json_tokener_parse(json_plain);
	if (!json_obj) {
		memcpy(reply_text, err_parse, sizeof(err_parse));
		ok = false;
		goto out;
	}


	if (!json_object_object_get_ex(json_obj, "ok", &res) || !res) {
		memcpy(reply_text, "Cannot find \"ok\" key from JSON API", 35);
		ok = false;
		goto out;
	}


	if (json_object_get_boolean(res))
		/*
		 * Successful!
		 */
		goto out;


	ok = false;
	if (!json_object_object_get_ex(json_obj, "description", &res)) {
		memcpy(reply_text, err_parse, sizeof(err_parse));
	} else {
		const char *str_desc = json_object_get_string(res);
		if (str_desc)
			snprintf(reply_text, RTB_SIZE, "Error: %s", str_desc);
		else
			memcpy(reply_text, err_parse, sizeof(err_parse));
	}

out:
	if (json_obj)
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
	if (ret)
		snprintf(reply_text, RTB_SIZE,
			 "Error: tga_kick_chat_member(): " PRERF, PREAR(-ret));
	else
		ok = process_json_msg(&thandle, reply_text);

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
		snprintf(reply_text, RTB_SIZE,
			 "Error: tga_restrict_chat_member(): " PRERF,
			 PREAR(-ret));
	} else {
		ok = process_json_msg(&thandle, reply_text);
	}

	tga_sdestroy(&thandle);
	return ok;
}


static bool do_pin(const struct gwbot_thread *thread, char *reply_text,
		    const tga_pin_cm_t *arg)
{
	int ret;
	bool ok = true;
	tga_handle_t thandle;

	tga_screate(&thandle, thread->state->cfg->cred.token);
	ret = tga_pin_chat_msg(&thandle, arg);

	if (ret) {
		snprintf(reply_text, RTB_SIZE,
			 "Error: tga_pin_chat_msg(): " PRERF, PREAR(-ret));
	} else {
		ok = process_json_msg(&thandle, reply_text);
	}

	tga_sdestroy(&thandle);
	return ok;
}


static bool do_unpin(const struct gwbot_thread *thread, char *reply_text,
		     const tga_unpin_cm_t *arg)
{
	int ret;
	bool ok = true;
	tga_handle_t thandle;

	tga_screate(&thandle, thread->state->cfg->cred.token);
	ret = tga_unpin_chat_msg(&thandle, arg);


	if (ret) {
		pr_err("tga_unpin_chat_msg() on do_pin(): " PRERF,
		       PREAR(-ret));
		snprintf(reply_text, RTB_SIZE,
			 "Error: tga_unpin_chat_msg(): " PRERF, PREAR(-ret));
	} else {
		ok = process_json_msg(&thandle, reply_text);
	}

	tga_sdestroy(&thandle);
	return ok;
}


static int do_del_msg(const struct gwbot_thread *thread, char *reply_text,
		      const tga_delete_msg_t *arg)
{
	int ret;
	bool ok = true;
	tga_handle_t thandle;

	tga_screate(&thandle, thread->state->cfg->cred.token);
	ret = tga_delete_msg(&thandle, arg);


	if (ret) {
		pr_err("tga_delete_msg() on do_pin(): " PRERF,
		       PREAR(-ret));
		snprintf(reply_text, RTB_SIZE,
			 "Error: tga_delete_msg(): " PRERF, PREAR(-ret));
	} else {
		ok = process_json_msg(&thandle, reply_text);
	}

	tga_sdestroy(&thandle);
	return ok;
}


static int kick_or_unban(const struct gwbot_thread *thread,
			 struct tgev *evt, uint64_t target_uid,
			 const char *reason, bool is_unban, bool silent)
{
	bool tmp;
	struct tgev *reply_to;
	uint64_t reply_to_msg_id;
	char reply_text[RTB_SIZE];

	tmp = do_unban(thread, reply_text, &(const tga_unban_cm_t){
		.chat_id = tge_get_chat_id(evt),
		.user_id = target_uid,
		.only_if_banned = is_unban
	});

	reply_to_msg_id = tge_get_msg_id(evt);
	if (!tmp)
		goto out;

	if (silent)
		return 0;

	reply_to = tge_get_reply_to(evt);
	if (reply_to) {
		size_t pos = 0, sps = sizeof(reply_text);
		const struct tgevi_from *fr = tge_get_from(reply_to);

		if (fr) {
			sps -= 18;
			pos  = gen_name_link(reply_text, sps, fr->id,
					     fr->first_name, fr->last_name);
			sps -= pos;
			memcpy(reply_text + pos, " has been ", 10);
			pos += 10;
		} else {
			memcpy(reply_text + pos, "(unknown) has been ", 19);
			pos += 19;
		}

		if (is_unban) {
			memcpy(reply_text + pos, "unbanned!", 10);
			pos += 9;
		} else {
			memcpy(reply_text + pos, "kicked!", 8);
			pos += 7;
		}


		if (reason) {
			memcpy(reply_text + pos, "\n<b>Message:</b> ", 18);
			pos += 17;
			sps -= 18;
			pos += htmlspecialchars(reply_text + pos, sps, reason,
						strnlen(reason, sps));
			reply_text[pos - 1] = '\0';
		}
	}

out:
	return send_reply(thread, evt, reply_text, reply_to_msg_id);
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
		size_t pos = 0, sps = sizeof(reply_text);
		const struct tgevi_from *fr = tge_get_from(reply_to);

		if (fr) {
			sps -= 18;
			pos  = gen_name_link(reply_text, sps, fr->id,
					     fr->first_name, fr->last_name);
			sps -= pos;
			memcpy(reply_text + pos, " has been ", 10);
			pos += 10;
		} else {
			memcpy(reply_text + pos, "(unknown) has been ", 19);
			pos += 19;
		}

		if (is_unmute) {
			memcpy(reply_text + pos, "unmuted!", 9);
			pos += 8;
		} else {
			memcpy(reply_text + pos, "muted!", 7);
			pos += 6;
		}


		if (reason) {
			memcpy(reply_text + pos, "\n<b>Message:</b> ", 18);
			pos += 17;
			sps -= 18;
			pos += htmlspecialchars(reply_text + pos, sps, reason,
						strnlen(reason, sps));
			reply_text[pos - 1] = '\0';
		}
	}

out:

	return send_reply(thread, evt, reply_text, reply_to_msg_id);
}


static int exec_adm_cmd_ban(const struct gwbot_thread *thread, struct tgev *evt,
			    uint64_t target_uid, const char *reason, bool silent)
{
	bool tmp;
	struct tgev *reply_to;
	uint64_t reply_to_msg_id;
	char reply_text[RTB_SIZE];
	int64_t chat_id = tge_get_chat_id(evt);

	tmp = do_kick(thread, reply_text, &(const tga_kick_cm_t){
		.chat_id = chat_id,
		.user_id = target_uid,
		.until_date = 0,
		.revoke_msg = true
	});

	reply_to_msg_id = tge_get_msg_id(evt);
	if (!tmp)
		goto out;

	if (silent) {
		/*
		 * Delete the command and the replied message.
		 */

		tmp = do_del_msg(thread, reply_text, &(const tga_delete_msg_t){
			.chat_id = chat_id,
			.message_id = reply_to_msg_id
		});

		reply_to = tge_get_reply_to(evt);
		if (!reply_to)
			return 0;

		tmp = do_del_msg(thread, reply_text, &(const tga_delete_msg_t){
			.chat_id = chat_id,
			.message_id = tge_get_msg_id(reply_to)
		});

		return 0;
	}

	reply_to = tge_get_reply_to(evt);
	if (reply_to) {
		size_t pos = 0, sps = sizeof(reply_text);
		const struct tgevi_from *fr = tge_get_from(reply_to);

		if (fr) {
			sps -= 18;
			pos  = gen_name_link(reply_text, sps, fr->id,
					     fr->first_name, fr->last_name);
			sps -= pos;
			memcpy(reply_text + pos, " has been banned!", 18);
			pos += 17;
		} else {
			memcpy(reply_text, "Banned success!", 16);
			pos  = 15;
			sps -= 15;
		}

		if (reason) {
			memcpy(reply_text + pos, "\n<b>Message:</b> ", 18);
			pos += 17;
			sps -= 18;
			pos += htmlspecialchars(reply_text + pos, sps, reason,
						strnlen(reason, sps));
			reply_text[pos - 1] = '\0';
		}
	}
out:
	return send_reply(thread, evt, reply_text, reply_to_msg_id);
}


static int exec_adm_cmd_unban(const struct gwbot_thread *thread,
			      struct tgev *evt, uint64_t target_uid,
			      const char *reason)
{
	bool silent = false;
	return kick_or_unban(thread, evt, target_uid, reason, true, silent);
}


static int exec_adm_cmd_kick(const struct gwbot_thread *thread,
			     struct tgev *evt, uint64_t target_uid,
			     const char *reason, bool silent)
{
	int ret = kick_or_unban(thread, evt, target_uid, reason, false, silent);

	if (!ret && silent) {
		char reply_text[RTB_SIZE];
		int64_t chat_id = tge_get_chat_id(evt);

		/*
		 * Delete the command and the replied message.
		 */
		struct tgev *reply_to = tge_get_reply_to(evt);

		do_del_msg(thread, reply_text, &(const tga_delete_msg_t){
			.chat_id = chat_id,
			.message_id = tge_get_msg_id(evt)
		});

		if (!reply_to)
			return 0;

		do_del_msg(thread, reply_text, &(const tga_delete_msg_t){
			.chat_id = chat_id,
			.message_id = tge_get_msg_id(reply_to)
		});
	}

	return ret;
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


static int exec_adm_cmd_pin(const struct gwbot_thread *thread, struct tgev *evt)
{
	bool tmp;
	char reply_text[RTB_SIZE];
	const char *rep = reply_text;
	uint64_t reply_to_msg_id;
	struct tgev *reply_to;

	reply_to = tge_get_reply_to(evt);
	reply_to_msg_id = tge_get_msg_id(reply_to);
	if (reply_to_msg_id == 0) {
		rep = "Please reply to message to be pinned!";
		goto out;
	}

	tmp = do_pin(thread, reply_text, &(const tga_pin_cm_t){
		.chat_id = tge_get_chat_id(evt),
		.message_id = reply_to_msg_id,
		.disable_notification = false
	});

	if (!tmp)
		goto out;

	snprintf(reply_text, RTB_SIZE, "Message %" PRIu64 " has been pinned!",
		 reply_to_msg_id);

out:
	return send_reply(thread, evt, rep, tge_get_msg_id(evt));
}


static int exec_adm_cmd_unpin(const struct gwbot_thread *thread, struct tgev *evt)
{
	bool tmp;
	char reply_text[RTB_SIZE];
	const char *rep = reply_text;
	uint64_t reply_to_msg_id;
	struct tgev *reply_to;

	reply_to = tge_get_reply_to(evt);
	reply_to_msg_id = tge_get_msg_id(reply_to);
	if (reply_to_msg_id == 0) {
		rep = "Please reply to message to be unpinned!";
		goto out;
	}

	tmp = do_unpin(thread, reply_text, &(const tga_unpin_cm_t){
		.chat_id = tge_get_chat_id(evt),
		.message_id = reply_to_msg_id
	});

	if (!tmp)
		goto out;

	snprintf(reply_text, RTB_SIZE, "Message %" PRIu64 " has been unpinned!",
		 reply_to_msg_id);

out:
	return send_reply(thread, evt, rep, tge_get_msg_id(evt));
}


static int exec_adm_cmd_delete(const struct gwbot_thread *thread, struct tgev *evt)
{
	bool tmp;
	char reply_text[RTB_SIZE];
	const char *rep = reply_text;
	uint64_t reply_to_msg_id;
	struct tgev *reply_to;

	reply_to = tge_get_reply_to(evt);
	reply_to_msg_id = tge_get_msg_id(reply_to);
	if (reply_to_msg_id == 0) {
		rep = "Please reply to message to be unpinned!";
		goto out;
	}

	tmp = do_del_msg(thread, reply_text, &(const tga_delete_msg_t){
		.chat_id = tge_get_chat_id(evt),
		.message_id = reply_to_msg_id
	});

	if (!tmp)
		goto out;

	tmp = do_del_msg(thread, reply_text, &(const tga_delete_msg_t){
		.chat_id = tge_get_chat_id(evt),
		.message_id = tge_get_msg_id(evt)
	});

	if (!tmp)
		goto out;

	snprintf(reply_text, RTB_SIZE, "Message %" PRIu64 " has been deleted!",
		 reply_to_msg_id);

out:
	return send_reply(thread, evt, rep, tge_get_msg_id(evt));
}


static inline void construct_report_reply(char *rep_p, size_t sps,
					  struct tga_chat_member *admins,
					  size_t admin_c)
{

	memcpy(rep_p, "Ping admins!\n", 14);
	sps   -= 13;
	rep_p += 13;

	for (size_t i = 0; i < admin_c; i++) {
		size_t tmp;
		struct tga_user *user = &admins[i].user;
		const char *fn = user->first_name;

		if (user->is_bot)
			continue;

		sps -= 6;
		tmp  = (size_t)snprintf(rep_p, sps,
					"<a href=\"tg://user?id=%" PRIu64 "\">",
					user->id);

		rep_p += tmp;
		sps   -= tmp;

		tmp = htmlspecialchars(rep_p, sps, fn, strnlen(fn, 0xfful));

		if (tmp > 0)
			tmp--;

		rep_p += tmp;
		sps   -= tmp;

		memcpy(rep_p, "</a> ", 6);
		rep_p += 5;
	}
}


static int exec_usr_cmd_report(const struct gwbot_thread *thread,
			       struct tgev *evt, struct tgev *reply_to)
{
	int ret;
	size_t sps;
	size_t admin_c = 0;
	tga_handle_t thandle;
	char reply_text[4096];
	char *rep_p = reply_text;
	json_object *json_obj = NULL;
	const char *json_plain = NULL;
	struct tga_chat_member *admins = NULL;

	sps = sizeof(reply_text);

	tga_screate(&thandle, thread->state->cfg->cred.token);
	ret = tga_get_chat_admins(&thandle, tge_get_chat_id(evt));
	if (ret) {
		snprintf(reply_text, sps, "Error: tga_get_chat_admins(): "
			 PRERF, PREAR(-ret));
		goto out;
	}


	json_plain = tge_get_res_body(&thandle);
	if (!json_plain) {
		ret = -EINVAL;
		snprintf(reply_text, sps, "Error: tge_get_res_body(): "
			 PRERF, PREAR(EINVAL));
		goto out;
	}


	json_obj = json_tokener_parse(json_plain);
	if (!json_obj) {
		ret = -EINVAL;
		snprintf(reply_text, sps, "Error: json_tokener_parse(): "
			 PRERF, PREAR(EINVAL));
		goto out;
	}


	ret = parse_tga_admins(json_obj, &admins, &admin_c);
	if (ret) {
		snprintf(reply_text, sps, "Error: parse_tga_admins(): "
			 PRERF, PREAR(-ret));
		goto out;
	}


	construct_report_reply(rep_p, sps, admins, admin_c);

out:
	if (json_obj)
		json_object_put(json_obj);

	free(admins);
	tga_sdestroy(&thandle);

	if (reply_to)
		evt = reply_to;

	return send_reply(thread, evt, reply_text, tge_get_msg_id(evt));
}


static bool strtolower_cp(char *dest, const char *src, size_t dst_len)
{
	size_t i = 0;

	while (true) {
		char c = src[i];

		if ((c == '\0') || is_ws(c))
			break;

		dest[i] = ('A' <= c && c <= 'Z') ? c + 32 : c;

		if (++i >= dst_len)
			return false;
	}
	dest[i] = '\0';
	return true;
}


static bool is_an_admin_mention(const char *tx)
{
	char yx[10];

	if (*tx != '@')
		return false;

	tx++;
	if (!strtolower_cp(yx, tx, sizeof(yx) - 1))
		return false;

	if (strncmp("admin", yx, 5))
		return false;

	tx += 5;

	/*
	 * Match also @admins
	 */
	if (*tx == 's')
		tx++;

	if ((*tx != '\0') && !is_ws(*tx))
		return false;

	return true;
}


int GWMOD_ENTRY_DEFINE(003_admin, const struct gwbot_thread *thread,
				     struct tgev *evt)
{
	char c;
	char yx[10];
	int ret = -ECANCELED;
	struct tgev *reply_to;
	mod_cmd_t cmd = CMD_NOP;
	uint64_t user_id, target_uid = 0;
	const char *reason = NULL, *tx = tge_get_text(evt);


	if (tx == NULL)
		return ret;


	c = *tx++;
	if ((c != '!') && (c != '/') && (c != '.') && (c != '~')) {

		if (is_an_admin_mention(tx - 1)) {
			cmd      = USR_CMD_REPORT;
			user_id  = tge_get_user_id(evt);
			reply_to = tge_get_reply_to(evt);
			goto out_run_priv_chk;
		}

		return ret;
	}

	if (!strtolower_cp(yx, tx, sizeof(yx) - 1))
		return ret;

	c =
	(!strncmp("ban",     yx, 3) && (tx += 3) && (cmd = ADM_CMD_BAN))     ||
	(!strncmp("sban",    yx, 4) && (tx += 4) && (cmd = ADM_CMD_SBAN))    ||
	(!strncmp("unban",   yx, 5) && (tx += 5) && (cmd = ADM_CMD_UNBAN))   ||
	(!strncmp("kick",    yx, 4) && (tx += 4) && (cmd = ADM_CMD_KICK))    ||
	(!strncmp("skick",   yx, 5) && (tx += 5) && (cmd = ADM_CMD_SKICK))   ||
	(!strncmp("warn",    yx, 4) && (tx += 4) && (cmd = ADM_CMD_WARN))    ||
	(!strncmp("mute",    yx, 4) && (tx += 4) && (cmd = ADM_CMD_MUTE))    ||
	(!strncmp("tmute",   yx, 5) && (tx += 5) && (cmd = ADM_CMD_TMUTE))   ||
	(!strncmp("unmute",  yx, 6) && (tx += 6) && (cmd = ADM_CMD_UNMUTE))  ||
	(!strncmp("pin",     yx, 3) && (tx += 3) && (cmd = ADM_CMD_PIN))     ||
	(!strncmp("unpin",   yx, 5) && (tx += 5) && (cmd = ADM_CMD_UNPIN))   ||
	(!strncmp("delete",  yx, 6) && (tx += 6) && (cmd = ADM_CMD_DELETE))  ||
	(!strncmp("del",     yx, 3) && (tx += 3) && (cmd = ADM_CMD_DELETE))  ||
	(!strncmp("report",  yx, 6) && (tx += 6) && (cmd = USR_CMD_REPORT))  ||
	(!strncmp("admin",   yx, 5) && (tx += 5) && (cmd = USR_CMD_REPORT))  ||
	(!strncmp("delvote", yx, 7) && (tx += 7) && (cmd = USR_CMD_DELVOTE));


	if (!c)
		return ret;

	user_id  = tge_get_user_id(evt);
	reply_to = tge_get_reply_to(evt);


	c = *tx;
	if (!c) {
		if (cmd != USR_CMD_REPORT && !reply_to)
			return ret;

		goto out_run;
	}


	if (!is_ws(c))
		return ret;


	/*
	 * Skip white space.
	 */
	while (is_ws(*tx))
		tx++;

	reason = tx;

out_run:
	if (!reply_to) {

		if (!reason) {

			if (cmd == USR_CMD_REPORT)
				goto out_run_priv_chk;

			return ret;
		}

		/*
		 * TODO: Parse the reason, it may contain
		 *       username, user_id, or other arguments.
		 */
		return ret;
	} else {
		target_uid = tge_get_user_id(reply_to);
	}


out_run_priv_chk:
	if ((cmd & ADMIN_CMD_BITS) && !is_privileged_user(thread, evt, user_id))
		return 0;


	switch (cmd) {
	case CMD_NOP:
		panic("Got CMD_NOP in admin module");
		abort();
	case ADM_CMD_BAN:
	case ADM_CMD_SBAN:
		ret = exec_adm_cmd_ban(thread, evt, target_uid, reason,
				       cmd == ADM_CMD_SBAN);
		break;
	case ADM_CMD_UNBAN:
		ret = exec_adm_cmd_unban(thread, evt, target_uid, reason);
		break;
	case ADM_CMD_KICK:
	case ADM_CMD_SKICK:
		ret = exec_adm_cmd_kick(thread, evt, target_uid, reason,
					cmd == ADM_CMD_SKICK);
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
		ret = exec_adm_cmd_pin(thread, evt);
		break;
	case ADM_CMD_DELETE:
		ret = exec_adm_cmd_delete(thread, evt);
		break;
	case ADM_CMD_UNPIN:
		ret = exec_adm_cmd_unpin(thread, evt);
		break;
	case USR_CMD_REPORT:
		ret = exec_usr_cmd_report(thread, evt, reply_to);
		break;
	case USR_CMD_DELVOTE:
		break;
	}

	return ret;
}
