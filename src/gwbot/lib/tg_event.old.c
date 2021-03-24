// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/lib/tg_event.c
 *
 *  Recieve message functions and logic for gwbot.
 *
 *  Copyright (C) 2021  aliwoto
 */

#include <json-c/json.h>
#include <errno.h>
#include <gwbot/lib/tg_event.h>

/**
 * check if the specified message is a gif message or not.
 */
bool is_gif_msg(const _tg_event_msg *msg)
{
        const char *animation;
        const char *document;
        struct json_object *parsed_json;
        struct json_object *animation_json;
        struct json_object *document_json;

        parsed_json = json_parse(msg->body);

        json_ex(parsed_json, TG_ANIMATION_ELEMENT, &animation_json);
        json_ex(parsed_json, TG_DOCUMENT_ELEMENT, &document_json);

        animation = json_get_string(animation_json);
        document = json_get_string(document_json);

        return animation != NULL && document != NULL;
}

/**
 * check if the specified message is a text message or not.
 */
bool is_text_msg(const _tg_event_msg *msg)
{
        const char *text;
        struct json_object *parsed_json;
        struct json_object *text_json;

        parsed_json = json_parse(msg->body);

        json_ex(parsed_json, TG_TEXT_ELEMENT, &text_json);

        text = json_get_string(text_json);

        return text != NULL;
}

/**
 * check if the specified message is a photo message or not.
 */
bool is_photo_msg(const _tg_event_msg *msg)
{
        const char *photo;
        struct json_object *parsed_json;
        struct json_object *photo_json;

        parsed_json = json_parse(msg->body);

        json_ex(parsed_json, TG_PHOTO_ELEMENT, &photo_json);

        photo = json_get_string(photo_json);

        return photo != NULL;
}

/* 
 * check if the current message is a replied message or not.
 * if the message under the element "reply_to_message"
 * in the json data (its body) is not null, then it means the
 * message is a replied message, otherwise it's not.
 */
bool is_replied_msg(const _tg_event_msg *msg) 
{
        const char *replied;
        struct json_object *parsed;
        struct json_object *replied_to;

        parsed = json_parse(msg->body);
        if (unlikely(parsed == NULL)) {
                pr_err("Json serialized argument was not in correct format!");
                return -EINVAL;
        }
        json_ex(parsed, TG_REPLY_TO_MESSAGE_ELEMENT, &replied_to);
        replied = json_get_string(replied_to);
        return replied != NULL;
}

/**
 * check if the specified message is a sticker message or not.
 */
bool is_sticker_msg(const _tg_event_msg *msg)
{
        const char *sticker;
        struct json_object *parsed_json;
        struct json_object *sticker_json;

        parsed_json = json_parse(msg->body);

        json_ex(parsed_json, TG_STICKER_ELEMENT, &sticker_json);

        sticker = json_get_string(sticker_json);

        return sticker != NULL;
}



/**
 * set the kind of the specified message.
 */
int set_msg_kind(_tg_event_msg *msg)
{
        if (is_text_msg(msg)) {
                msg->msg_kind = TEXT_MSG;
                return 0;
        } else if (is_sticker_msg(msg)) {
                msg->msg_kind = STICKER_MSG;
                return 0;
        } else if (is_gif_msg(msg)) {
                msg->msg_kind = GIF_MSG;
                return 0;
        } else if (is_photo_msg(msg)) {
                msg->msg_kind = PHOTO_MSG;
                return 0;
        } else {
                msg->msg_kind = NONE;
                pr_err("Message argument was not in correct format!");
                return -EINVAL;
        }
        /* TODO:
        *       adding new format for messages.
        */
}

/*
 * load the event message.
 * please notice that none of the args should be null.
 */
int tg_event_msg_load(const char *json, _tg_event_msg *msg)
{
        if (unlikely(json == NULL || msg == NULL)) {
                pr_err("Arguments should NOT be NULL!");
                return -EINVAL;
        }
        
        msg->body = json;
        /**
         * WARNING: don't use this function to load a 
         * message which is under the element of reply_to_message
         * in the json serialized data. 
         * they are not main message!
         */
        msg->is_main_msg = true;
        set_msg_kind(msg);
        return 0;
}

/*
* load the event struct from specified json 
* which is from telegram.
* first arg is the json serialized data from telegram servers,
* and it can NOT be NULL.
* the second arg is event data structure,
* which can be NULL.
*/
int tg_event_load(const char *json, _tg_event *event)
{
        int ret;
        struct json_object *parsed;
        struct json_object *update_id;
        struct json_object *tg_msg;
        _tg_event_msg *msg = &event->tg_msg;

        if (unlikely(json == NULL || event == NULL)) {
                pr_err("Json serialized argument should NOT be NULL!");
                return -EINVAL;
        }

        parsed = json_parse(json);
        if (unlikely(parsed == NULL))
        {
                pr_err("Json serialized argument was not in correct format!");
                return -EINVAL;
        }
        
        json_ex(parsed, TG_UPDATE_ID_ELEMENT, &update_id);
        json_ex(parsed, TG_MESSAGE_ELEMENT, &tg_msg);
        
        event->update_id = (uint64_t)atoi(json_get_string(update_id));

        return tg_event_msg_load(json_get_string(tg_msg), &event->tg_msg);
}