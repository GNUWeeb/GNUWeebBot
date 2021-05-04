// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_api/restrict_chat_member.h
 *
 *  Restrict chat member API for GNUWeebBot
 *
 *  Copyright (C) 2021  Komori Kuzuyu
 */

#ifndef GWBOT__LIB__TG_API__RESTRICT_CHAT_MEMBER_H
#define GWBOT__LIB__TG_API__RESTRICT_CHAT_MEMBER_H

#include <gwbot/lib/tg_api.h>


typedef struct _tga_res_perm_t {
	bool		can_send_messages;
	bool		can_send_media_messages;
	bool		can_send_polls;
	bool		can_send_other_messages;
	bool		can_add_web_page_previews;
	bool		can_change_info;
	bool		can_invite_users;
	bool		can_pin_messages;
} tga_res_perm_t;


typedef struct _tga_restrict_cm_t {
	int64_t		chat_id;
	uint64_t	user_id;
	time_t		until_date;
	tga_res_perm_t	permissions;
} tga_restrict_cm_t;


int tga_restrict_chat_member(tga_handle_t *handle, const tga_restrict_cm_t *ctx);


#endif /* #ifndef GWBOT__LIB__TG_API__RESTRICT_CHAT_MEMBER_H */
