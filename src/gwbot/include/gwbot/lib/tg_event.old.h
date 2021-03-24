// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event.h
 *
 *  Recieve message header for gwbot.
 *
 *  Copyright (C) 2021  aliwoto
 */

#ifndef GWBOT__LIB__TG_EVENT_H
#define GWBOT__LIB__TG_EVENT_H

#include <stdlib.h>
#include <gwbot/base.h>


#define TG_UPDATE_ID_ELEMENT            "update_id"
#define TG_MESSAGE_ELEMENT              "message"
#define TG_MESSAGE_ID_ELEMENT           "message_id"
#define TG_FROM_ELEMENT                 "from"
#define TG_OWNER_ID_ELEMENT             "id"
#define TG_IS_BOT_ELEMENT               "is_bot"
#define TG_USERNAME_ELEMENT             "username"
#define TG_CHAT_ELEMENT                 "chat"
#define TG_TITLE_ELEMENT                "title"
#define TG_TYPE_ELEMENT                 "type"
#define TG_DATE_ELEMENT                 "date"
#define TG_TEXT_ELEMENT                 "text"
#define TG_ENTITIES_ELEMENT             "entities"
#define TG_OFFSET_ELEMENT               "offset"
#define TG_LENGTH_ELEMENT               "length"
#define TG_REPLY_TO_MESSAGE_ELEMENT     "reply_to_message"
#define TG_STICKER_ELEMENT              "sticker"
#define TG_ANIMATION_ELEMENT            "animation"
#define TG_DOCUMENT_ELEMENT             "document"
#define TG_PHOTO_ELEMENT                "photo"

#define json_parse(X) json_tokener_parse(X)
#define json_ex(X1, X2, X3) json_object_object_get_ex(X1, X2, X3)

#define json_get_string(X) json_object_get_string(X)
#define json_get_int(X) json_object_get_int(X)

/* 
 * The kind of the message event.
 */
typedef enum _msg_kind_t {
        NONE            = 0,
        GIF_MSG         = 1,
	TEXT_MSG        = 2,
	PHOTO_MSG       = 3,
        VIDEO_MSG       = 4,
	STICKER_MSG     = 5,
} _msg_kind_t;


/* 
 * The event message.
 */
typedef struct _tg_event_msg {
        const char      *body;
        bool            is_main_msg;
        bool            is_forwarded;
        _msg_kind_t     msg_kind;
} _tg_event_msg;



/* 
 * The Event.
 */
typedef struct _tg_event {
        uint64_t        update_id;
        _tg_event_msg   tg_msg;

} _tg_event;


/* 
 * The event sender.
 */
typedef struct _tg_msg_sender {
        /* TODO */
} _tg_msg_sender;


/* 
 * The chat room of the message event.
 */
typedef struct _tg_event_chat {
        /* TODO */
} _tg_event_chat;

bool is_gif_msg(const _tg_event_msg *msg);
bool is_text_msg(const _tg_event_msg *msg);
bool is_photo_msg(const _tg_event_msg *msg);
bool is_replied_msg(const _tg_event_msg *msg);
bool is_sticker_msg(const _tg_event_msg *msg);


int set_msg_kind(_tg_event_msg *msg);
int tg_event_load(const char *json, _tg_event *event);
int tg_event_msg_load(const char *json, _tg_event_msg *msg);


#endif /* #ifndef GWBOT__LIB__TG_EVENT_H */