/* SPDX-License-Identifier: GPL-2.0-only
 *
 *  src/gwbot/include/gwbot/lib/tg_api/delete_chat.h
 *
 *  delete chat message API for GNUWeebBot
 *
 *  Copyright (C) 2021  shashi918
 */

#ifndef GWBOT__LIB__TG_API__DELETE_MESSAGE_H
#define GWBOT__LIB__TG_API__DELETE_MESSAGE_H

#include <gwbot/lib/tg_api.h>


typedef struct _tga_delete_chat_t {
        int64_t		chat_id;
	uint64_t	message_id;
} tga_delete_chat_t;


int tga_delete_chat(tga_handle_t *handle, const tga_delete_chat_t *ctx);


#endif /* ifdef GWBOT__LIB__TG_API__DELETE_CHAT_MESSAGE_H */
