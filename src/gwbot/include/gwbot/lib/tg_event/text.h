// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event/text.h
 *
 *  Text message header for events
 *
 *  Copyright (C) 2021  aliwoto
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
        uint64_t        date;
        _tg_msg_sender  sender;
        _tg_event_chat  chat_room;
        _tg_msg_entity  *entity;
        _tg_text_msg    *reply_to_msg;
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

int convert_text_msg(const _tg_event_msg *msg, _tg_text_msg *text_msg);


static inline _tg_text_msg *tg_get_text_msg(const _tg_event_msg *msg)
{
        int ret;
        _tg_text_msg *text_msg;


        if (msg->msg_kind != TEXT_MSG) {
                pr_err("Trying to get a text from non-text kind event!");
                return NULL;
        }

        ret = convert_text_msg(msg, text_msg);
        if (ret != 0) {
                pr_err("argument message data was not in correct format.")
                return NULL;
        }
        return text_msg;
}



#endif /* #ifndef GWBOT__LIB__TG_EVENT__TEXT_H */