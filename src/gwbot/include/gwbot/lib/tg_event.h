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



typedef enum _tgev_type_t {
	TGEV_UNKNOWN	= (1 << 0),
	TGEV_TEXT	= (1 << 1),
	TGEV_PHOTO	= (1 << 2),
	TGEV_STICKER	= (1 << 3),
} tgev_type_t;


typedef enum _tgev_chat_type_t {
	TGEV_CHAT_GROUP 	= (1 << 0),
	TGEV_CHAT_SUPERGROUP 	= (1 << 1),
	TGEV_CHAT_PRIVATE 	= (1 << 2),
} tgev_chat_type_t;


struct tgevi_file {
	const char 		*file_id;
	const char		*file_unique_id;
	size_t			file_size;
	uint16_t		width;
	uint16_t		height;
};



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
	int64_t			id;
	const char		*title;
	const char		*username;
	const char		*first_name;
	const char		*last_name;
	tgev_chat_type_t	type;
	bool			all_admins;
};


struct tgevi_entity {
	uint16_t	offset;
	uint16_t	length;
	struct_pad(0, 4);
	const char	*type;
};



static __always_inline int parse_tgevi_file(json_object *jfile,
					    struct tgevi_file *file)
{
	json_object *res;

	if (unlikely(!json_object_object_get_ex(jfile, "file_id", &res))) {
		pr_err("Cannot find \"file_id\" key in \"file\"");
		return -EINVAL;
	}
	file->file_id = json_object_get_string(res);



	if (unlikely(!json_object_object_get_ex(jfile, "file_unique_id", &res))) {
		pr_err("Cannot find \"file_unique_id\" key in \"file\"");
		return -EINVAL;
	}
	file->file_unique_id = json_object_get_string(res);


	if (unlikely(!json_object_object_get_ex(jfile, "file_size", &res))) {
		pr_err("Cannot find \"file_size\" key in \"file\"");
		return -EINVAL;
	}
	file->file_size = json_object_get_uint64(res);



	if (unlikely(!json_object_object_get_ex(jfile, "width", &res))) {
		pr_err("Cannot find \"width\" key in \"file\"");
		return -EINVAL;
	}
	file->width = (uint16_t)json_object_get_uint64(res);


	if (unlikely(!json_object_object_get_ex(jfile, "height", &res))) {
		pr_err("Cannot find \"height\" key in \"file\"");
		return -EINVAL;
	}
	file->height = (uint16_t)json_object_get_uint64(res);

	return 0;
}






static __always_inline int parse_tgevi_entities(json_object *jentities,
					        uint16_t entity_c,
					        struct tgevi_entity **entities)
{
	json_object *entity, *tmp;
	struct tgevi_entity *entities_tmp;

	entities_tmp = calloc(entity_c, sizeof(*entities_tmp));
	if (unlikely(entities_tmp == NULL)) {
		pr_err("calloc(): " PRERF, PREAR(ENOMEM));
		return -ENOMEM;
	}


	for (uint16_t i = 0; i < entity_c; i++) {
		struct tgevi_entity *eptr = &entities_tmp[i];
		entity = json_object_array_get_idx(jentities, i);

		if (!json_object_object_get_ex(entity, "offset", &tmp))
			continue;
		eptr->offset = (uint16_t)json_object_get_uint64(tmp);

		if (!json_object_object_get_ex(entity, "length", &tmp))
			continue;
		eptr->length = (uint16_t)json_object_get_uint64(tmp);

		if (!json_object_object_get_ex(entity, "type", &tmp))
			continue;
		eptr->type = json_object_get_string(tmp);
	}

	*entities = entities_tmp;
	return 0;
}





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
	const char *type;

	if (unlikely(!json_object_object_get_ex(jchat, "id", &res))) {
		pr_err("Cannot find \"id\" on key \"chat\"");
		return -EINVAL;
	}
	chat->id = json_object_get_int64(res);

	if (unlikely(!json_object_object_get_ex(jchat, "type", &res))) {
		pr_err("Cannot find \"type\" on key \"chat\"");
		return -EINVAL;
	}
	type = json_object_get_string(res);
	switch (type[0]) {
	case 's':
		chat->type = TGEV_CHAT_SUPERGROUP;
		break;
	case 'g':
		chat->type = TGEV_CHAT_GROUP;
		break;
	case 'p':
		chat->type = TGEV_CHAT_PRIVATE;
		break;
	default:
		pr_err("Unknown \"type\" on key \"chat\": \"%s\"", type);
		return -EINVAL;
	}
	

	if (chat->type == TGEV_CHAT_PRIVATE) {
		chat->title = NULL;
		if (unlikely(!json_object_object_get_ex(jchat, "first_name",
								&res))) {
			pr_err("Cannot find \"first_name\" on key \"chat\"");
			return -EINVAL;
		}
		chat->first_name = json_object_get_string(res);
		if (unlikely(!json_object_object_get_ex(jchat, "last_name",
		 						&res))) {
			/* `last_name` is not mandatory */
			chat->last_name = NULL;
		} else {
			chat->last_name = json_object_get_string(res);
		}
	} else {
		if (chat->type == TGEV_CHAT_GROUP) {
			if (unlikely(!json_object_object_get_ex(jchat, 
					"all_members_are_administrators", 
								&res))) {
				pr_err("Cannot find \"all_admins\" "
						"on key \"chat\"");
				return -EINVAL;

			}
			chat->all_admins = 
				json_object_get_boolean(res)  ? true : false;
		}
		
		if (unlikely(!json_object_object_get_ex(jchat, "title", 
								&res))) {
			/*
			 * `title` is mandatory, bacause the type is
			 * not private.
			 */
			pr_err("Cannot find \"title\" on key \"chat\"");
			return -EINVAL;

		}
		chat->title = json_object_get_string(res);
	}
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


int parse_message(json_object *json_obj, struct tgev *evt);
int tg_event_load_str_len(const char *json_str, size_t length, struct tgev *evt);
int tg_event_load_str(const char *json_str, struct tgev *evt);
void tg_event_destroy(struct tgev *evt);


#endif /* #ifndef GWBOT__LIB__TG_EVENT_H */
