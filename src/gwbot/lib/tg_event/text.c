// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/lib/tg_event/text.c
 *
 *  Text message functions and logic code file for events.
 *
 *  Copyright (C) 2021  aliwoto
 */

#include <errno.h>
#include <json-c/json.h>
#include <gwbot/lib/tg_event.h>
#include <gwbot/lib/tg_event/text.h>

int convert_text_msg(const _tg_event_msg *msg, _tg_text_msg *text_msg)
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
}