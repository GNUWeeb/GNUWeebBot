// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event.h
 *
 *  String helpers header for TeaVPN2
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__TG_EVENT_H
#define GWBOT__LIB__TG_EVENT_H

#include <time.h>
#include <stdlib.h>
#include <gwbot/base.h>
#include <gwbot/lib/string.h>
#include <json-c/json.h>


/*
 *
 * `tgev` means `tg event` (Telegram Event)
 * `tgevi` means `tg event info` (Telegram Event Info)
 *
 */


struct tgevi_from {
	uint64_t	id;
	const char	*first_name;
	const char	*last_name;
	const char	*username;
	char		lang[4];
	bool		is_bot;
	struct_pad(0, 3);
};


struct tgevi_chat {
	int64_t		id;
	const char	*title;
	const char	*type;
	const char	*username;
};


struct tgevi_entity {
	uint16_t	offset;
	uint16_t	length;
	struct_pad(0, 4);
	const char	*type;
};


typedef enum _tgev_type_t {
	TGEV_UNKNOWN	= 0,
	TGEV_TEXT	= 1,
	TGEV_PHOTO	= 2,
	TGEV_STICKER	= 3,
} tgev_type_t;

static __always_inline int parse_tgevi_from(json_object *jfrom,
					    struct tgevi_from *from)
{
	json_object *res;

	if (unlikely(!json_object_object_get_ex(jfrom, "id", &res))) {
		pr_err("Cannot find \"id\" on key \"from\"");
		return -EINVAL;
	}
	from->id = json_object_get_uint64(res);



	if (unlikely(!json_object_object_get_ex(jfrom, "first_name", &res))) {
		pr_err("Cannot find \"first_name\" on key \"from\"");
		return -EINVAL;
	}
	from->first_name = json_object_get_string(res);



	if (unlikely(!json_object_object_get_ex(jfrom, "last_name", &res))) {
		/* `last_name` is not mandatory */
		from->last_name = NULL;
	} else {
		from->last_name = json_object_get_string(res);
	}



	if (unlikely(!json_object_object_get_ex(jfrom, "username", &res))) {
		/* `username` is not mandatory */
		from->username = NULL;
	} else {
		from->username = json_object_get_string(res);
	}



	if (unlikely(!json_object_object_get_ex(jfrom, "language_code", &res))) {
		/* `lang` is not mandatory */
		from->lang[0] = '\0';
	} else {
		sane_strncpy(from->lang, json_object_get_string(res),
			     sizeof(from->lang));
	}



	if (unlikely(!json_object_object_get_ex(jfrom, "is_bot", &res))) {
		/* `is_bot` is not mandatory */
		from->is_bot = false;
	} else {
		from->is_bot = json_object_get_boolean(res) ? true : false;
	}


	return 0;
}



static __always_inline int parse_tgevi_chat(json_object *jchat,
					    struct tgevi_chat *chat)
{
	json_object *res;

	if (unlikely(!json_object_object_get_ex(jchat, "id", &res))) {
		pr_err("Cannot find \"id\" on key \"chat\"");
		return -EINVAL;
	}
	chat->id = json_object_get_int64(res);



	if (unlikely(!json_object_object_get_ex(jchat, "title", &res))) {
		pr_err("Cannot find \"title\" on key \"chat\"");
		return -EINVAL;
	}
	chat->title = json_object_get_string(res);



	if (unlikely(!json_object_object_get_ex(jchat, "type", &res))) {
		pr_err("Cannot find \"type\" on key \"chat\"");
		return -EINVAL;
	}
	chat->type = json_object_get_string(res);



	if (unlikely(!json_object_object_get_ex(jchat, "username", &res))) {
		/* `username` is not mandatory */
		chat->username = NULL;
	} else {
		chat->username = json_object_get_string(res);
	}


	return 0;
}



#define INCLUDE_SUB_TG_EVENT
#include <gwbot/lib/tg_event/text.h>
#include <gwbot/lib/tg_event/photo.h>
#include <gwbot/lib/tg_event/sticker.h>
#undef INCLUDE_SUB_TG_EVENT


struct tgev {
	/* 
	 * We need to hold the JSON object to release
	 * the resource, as it holds the reference
	 * to `const char *`.
	 *
	 * It hurts the performance if we allocate
	 * new memory and copy it. So let it here.
	 */
	json_object	*json;

	uint64_t	update_id;
	tgev_type_t	type;
	struct_pad(0, 4);
	union {
		struct tgev_text	msg_text;
		struct tgev_photo	msg_photo;
		struct tgev_sticker	msg_sticker;
	};
};


#define INCLUDE_SUB_TG_EVENT
#define SUB_TG_EVENT_CIRCULAR_INLINE
#include <gwbot/lib/tg_event/text.h>
#include <gwbot/lib/tg_event/photo.h>
#include <gwbot/lib/tg_event/sticker.h>
#undef SUB_TG_EVENT_CIRCULAR_INLINE
#undef INCLUDE_SUB_TG_EVENT


int tg_event_load_str_len(const char *json_str, size_t length, struct tgev *evt);
int tg_event_load_str(const char *json_str, struct tgev *evt);
void tg_event_destroy(struct tgev *evt);


#endif /* #ifndef GWBOT__LIB__TG_EVENT_H */
