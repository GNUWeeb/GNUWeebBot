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

typedef enum _mod_cmd_t {
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


static inline bool is_ws(char c)
{
	return (c == ' ') || (c == '\n') || (c == '\t') || (c == '\r');
}

static bool check_is_sudoer(uint64_t user_id)
{
	/*
	 * TODO: Use binary search for large sudoers list.
	 */
	static uint64_t sudoers[] = {
		701123895ull,	// lappretard
		1213668471ull,	// nrudesu
		133862899ull,	// ryne4s,
		243692601ull,	// ammarfaizi2
	};

	for (size_t i = 0; i < (sizeof(sudoers) / sizeof(*sudoers)); i++) {
		if (sudoers[i] == user_id)
			return true;
	}

	return false;
}


// static int send_eperm(const struct gwbot_thread *thread, struct tgev *evt)
// {
// 	int ret;
// 	tga_handle_t thandle;
// 	static const char reply_text[] 
// 		= "You don't have permission to execute this command!";

// 	tga_screate(&thandle, thread->state->cfg->cred.token);
// 	ret = tga_send_msg(&thandle, &(const tga_send_msg_t){
// 		.chat_id          = tge_get_chat_id(evt),
// 		.reply_to_msg_id  = tge_get_msg_id(evt),
// 		.text             = reply_text,
// 		.parse_mode       = PARSE_MODE_OFF
// 	});
// 	tga_sdestroy(&thandle);

// 	if (ret) {
// 		pr_err("tga_send_msg() on send_reply_text(): " PRERF,
// 		       PREAR(-ret));
// 	}

// 	return 0;
// }


int GWMOD_ENTRY_DEFINE(003_admin, const struct gwbot_thread *thread,
				     struct tgev *evt)
{
	char c;
	uint64_t user_id;
	bool is_sudoer = false;
	int ret = -ECANCELED;
	const char *tx = tge_get_text(evt), *reason = NULL;
	struct tgev *reply_to;
	mod_cmd_t cmd;

	if (tx == NULL)
		goto out;

	c = *tx++;
	if ((c != '!') && (c != '/') && (c != '.') && (c != '~'))
		goto out;


	c =
	(!strncmp("ban",     tx, 3) && (tx += 3) && (cmd = ADM_CMD_BAN))     ||
	(!strncmp("unban",   tx, 5) && (tx += 5) && (cmd = ADM_CMD_UNBAN))   ||
	(!strncmp("kick",    tx, 4) && (tx += 4) && (cmd = ADM_CMD_KICK))    ||
	(!strncmp("warn",    tx, 4) && (tx += 4) && (cmd = ADM_CMD_WARN))    ||
	(!strncmp("mute",    tx, 4) && (tx += 4) && (cmd = ADM_CMD_MUTE))    ||
	(!strncmp("tmute",   tx, 5) && (tx += 5) && (cmd = ADM_CMD_TMUTE))   ||
	(!strncmp("unmute",  tx, 6) && (tx += 6) && (cmd = ADM_CMD_UNMUTE))  ||
	(!strncmp("pin",     tx, 3) && (tx += 3) && (cmd = ADM_CMD_PIN))     ||
	(!strncmp("report",  tx, 6) && (tx += 6) && (cmd = USR_CMD_REPORT))  ||
	(!strncmp("delvote", tx, 7) && (tx += 7) && (cmd = USR_CMD_DELVOTE));

	if (!c)
		goto out;


	reply_to = tge_get_reply_to(evt);
	user_id  = tge_get_user_id(evt);

	c = *tx;
	if (c == '\0') {

		if (!reply_to)
			/*
			 * TODO: Send argument required, or reply required.
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
			/*
			 * TODO: Parse the reason, it may contain
			 *       username, user_id, etc.
			 */
			goto out;
		}
	}


	if (cmd & ADMIN_BITS) {
		/*
		 * This command requires administrator privilege
		 */

		is_sudoer = check_is_sudoer(user_id);
		if (!is_sudoer)
			/*
			 * TODO: Send permission denied
			 * TODO: Group admin check
			 */
			goto out;

	}



	printf("reason = %s\n", reason);
	printf("cmd = %u\n", cmd);
out:
	return ret;
}
