// SPDX-License-Identifier: GPL-2.0
/*
 *  src/gwbot/include/gwbot/lib/tg_api/delete_message.h
 *
 *  Send message API for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__TG_API__DELETE_MESSAGE_H
#define GWBOT__LIB__TG_API__DELETE_MESSAGE_H

#include <gwbot/lib/tg_api.h>


typedef struct _tga_delete_msg_t {
	int64_t		chat_id;
	uint64_t	message_id;
} tga_delete_msg_t;


int tga_delete_msg(tga_handle_t *handle, const tga_delete_msg_t *ctx);


#endif /* #ifndef GWBOT__LIB__TG_API__DELETE_MESSAGE_H */
