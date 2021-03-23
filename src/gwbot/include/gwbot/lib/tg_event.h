// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event.h
 *
 *  Recieve message header for gwbot
 *
 *  Copyright (C) 2021  aliwoto
 */

#ifndef GWBOT__LIB__TG_EVENT_H
#define GWBOT__LIB__TG_EVENT_H

#include <stdlib.h>
#include <gwbot/base.h>

/* 
 * The kind of the message event.
 */
typedef enum _msg_kind_t {
	TEXT_MSG        = 0,
	PHOTO_MSG       = 1,
	STICKER_MSG     = 2
} _msg_kind_t;


/* 
 * The Event.
 */
typedef struct _tg_event {
        uint64_t        update_id;
        _tg_event_msg   tg_msg;

} _tg_event;


/* 
 * The event message.
 */
typedef struct _tg_event_msg {
        bool            is_main_msg;
        const char      *body;
        _msg_kind_t     msg_kind;
} _tg_event_msg;


/* 
 * The event sender.
 */
typedef struct _tg_msg_sender {
    int64_t msg_id;


} _tg_msg_sender;


/* 
 * The chat room of the message event.
 */
typedef struct _tg_event_chat {
    int64_t msg_id;


} _tg_event_chat;










#endif /* #ifndef GWBOT__LIB__TG_EVENT_H */