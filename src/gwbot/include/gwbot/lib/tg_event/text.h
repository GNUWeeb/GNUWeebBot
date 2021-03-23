// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_api/send_message.h
 *
 *  String helpers header for TeaVPN2
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__TG_EVENT__TEXT_H
#define GWBOT__LIB__TG_EVENT__TEXT_H

#include <gwbot/lib/tg_event.h>

/* 
 * The kind of the message event.
 */
typedef enum _tg_text_type_t {
	ITALIC_TEXT_MSG         = 0,
	BOLD_TEXT_MSG           = 1,
	COMMAND_TEXT_MSG        = 2
} _tg_text_type_t;

/* 
 * The event text message.
 */
typedef struct _tg_text_msg {
        const char      *text;
        bool            has_entities;
        bool            is_replied;
        uint64_t        message_id;
        _tg_msg_sender  sender;
        _tg_event_chat  chat_room;
        uint64_t        date;
        _tg_msg_entity  entity;
} _tg_text_msg;


/* 
 * The entity of the text message.
 */
typedef struct _tg_msg_entity
{
        uint32_t        offset;
        uint32_t        length;
        _tg_text_type_t text_type;
} _tg_msg_entity;



static inline _tg_text_msg *tg_get_text_msg(_tg_event_msg *msg)
{
        if (msg->msg_kind != TEXT_MSG)
        {
                pr_err("ERR: TRYING TO GET A TEXT FROM NON-TEXT KIND EVENT!");
                return NULL;
        }
        /*
        * TODO:
        * we should convert the _tg_event_msg to a _tg_text_msg here.
        * 
        */
}



#endif /* #ifndef GWBOT__LIB__TG_EVENT__TEXT_H */