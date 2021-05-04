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
	TGEV_GIF	= (1 << 4),
} tgev_type_t;


typedef enum _tgev_chat_type_t {
	TGEV_CHAT_GROUP 	= (1 << 0),
	TGEV_CHAT_SUPERGROUP 	= (1 << 1),
	TGEV_CHAT_PRIVATE 	= (1 << 2),
} tgev_chat_type_t;



struct tgevi_file {
	const char		*file_name;
	const char 		*file_id;
	const char		*file_unique_id;
	const char		*mime_type;
	size_t			file_size;
	uint16_t		width;
	uint16_t		height;
};

struct tgevi_media {
	const char 		*file_id;
	const char		*file_name;
	const char		*mime_type;
	const char		*title;
	const char		*performer;
	const char		*file_unique_id;
	size_t			file_size;
	uint32_t		duration;
	uint16_t		width;
	uint16_t		height;
	struct tgevi_file	thumb;
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

	if (unlikely(!json_object_object_get_ex(jfile, "file_name", &res))) {
		/* `file_name` is not mandatory */
		file->file_name = NULL;
	} else {
		file->file_name = json_object_get_string(res);
	}
	
	if (unlikely(!json_object_object_get_ex(jfile, "mime_type", &res))) {
		/* `mime_type` is not mandatory */
		file->mime_type = NULL;
	} else {
		file->mime_type = json_object_get_string(res);
	}

	if (unlikely(!json_object_object_get_ex(jfile, "file_id", &res))) {
		pr_err("Cannot find \"file_id\" key in \"file\"");
		return -EINVAL;
	}
	file->file_id = json_object_get_string(res);

	if (unlikely(!json_object_object_get_ex(jfile, 
					"file_unique_id", &res))) {
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
		/* `width` is not mandatory */
		file->width  = 0u;
		file->height = 0u;
	} else {
		file->width = (uint16_t)json_object_get_uint64(res);
		if (unlikely(!json_object_object_get_ex(jfile, "height", 
								&res))) {
			/*
		 	* `height` is originally not mandatory, but since
		 	* there is `width` parameter, when there SHOULD be
		 	* a height parameter in the json data.
		 	*/
			pr_err("Cannot find \"height\" key in \"file\"");
			return -EINVAL;
		}
		file->height = (uint16_t)json_object_get_uint64(res);
	}
	


	return 0;
}


static __always_inline int parse_tgevi_media(json_object *jmdia,
					       struct tgevi_media *imdia)
{
	int ret;
	json_object *res;

	if (unlikely(!json_object_object_get_ex(jmdia, "file_name", &res))) {
		pr_err("Cannot find \"file_name\" key in \"media\"");
		return -EINVAL;
	}
	imdia->file_name = json_object_get_string(res);


	if (unlikely(!json_object_object_get_ex(jmdia, "mime_type", &res))) {
		pr_err("Cannot find \"mime_type\" key in \"media\"");
		return -EINVAL;
	}
	imdia->mime_type = json_object_get_string(res);

	if (unlikely(!json_object_object_get_ex(jmdia, "duration", &res))) {
		/* `title` is not mandatory */
		imdia->duration = 0;
	} else {
		imdia->duration = (uint32_t)json_object_get_uint64(res);
	}


	if (unlikely(!json_object_object_get_ex(jmdia, "width", &res))) {
		/* `width` is not mandatory */
		imdia->width  = 0u;
		imdia->height = 0u;
	} else {
		imdia->width = (uint16_t)json_object_get_uint64(res);
		if (unlikely(!json_object_object_get_ex(jmdia, "height", 
								&res))) {
			/*
		 	* `height` is originally not mandatory, but since
		 	* there is `width` parameter, when there SHOULD be
		 	* a height parameter in the json data.
		 	*/
			pr_err("Cannot find \"height\" key in \"file\"");
			return -EINVAL;
		}
		imdia->height = (uint16_t)json_object_get_uint64(res);
	}
	
	if (unlikely(!json_object_object_get_ex(jmdia, "title", &res))) {
		/* `title` is not mandatory */
		imdia->title = NULL;
	} else {
		imdia->title = json_object_get_string(res);
	}

	if (unlikely(!json_object_object_get_ex(jmdia, "performer", &res))) {
		/* `performer` is not mandatory */
		imdia->performer = NULL;
	} else {
		imdia->performer = json_object_get_string(res);
	}


	if (unlikely(!json_object_object_get_ex(jmdia, "file_id", &res))) {
		pr_err("Cannot find \"file_id\" key in \"media\"");
		return -EINVAL;
	}
	imdia->file_id = json_object_get_string(res);


	if (unlikely(!json_object_object_get_ex(jmdia, "file_unique_id",
						&res))) {
		pr_err("Cannot find \"file_unique_id\" key in \"media\"");
		return -EINVAL;
	}
	imdia->file_unique_id = json_object_get_string(res);


	if (unlikely(!json_object_object_get_ex(jmdia, "file_size", &res))) {
		pr_err("Cannot find \"file_size\" key in \"file\"");
		return -EINVAL;
	}
	imdia->file_size = json_object_get_uint64(res);

	if (unlikely(!json_object_object_get_ex(jmdia, "thumb", &res))) {
		/* `thumb` is not mandatory */
	} else {
		ret = parse_tgevi_file(jmdia, &imdia->thumb);
		if (unlikely(ret != 0))
			return ret;
		
	}
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
		const char *lang = json_object_get_string(res);
		if (lang)
			sane_strncpy(from->lang, lang, sizeof(from->lang));
		else
			from->lang[0] = '\0';
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
	out_missing_chat_type:
		pr_err("Cannot find \"type\" on key \"chat\"");
		return -EINVAL;
	}

	type = json_object_get_string(res);
	if (unlikely(!type))
		goto out_missing_chat_type;

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
#include <gwbot/lib/tg_event/gif.h>
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
	bool		is_replied_node;
	struct_pad(0, 3);
	union {
		struct tgev_gif		msg_gif;
		struct tgev_text	msg_text;
		struct tgev_photo	msg_photo;
		struct tgev_sticker	msg_sticker;
	};
};


int parse_message(json_object *json_obj, struct tgev *evt);
int tg_event_load_str_len(const char *json_str, size_t length, struct tgev *evt);
int tg_event_load_str(const char *json_str, struct tgev *evt);
void tg_event_destroy(struct tgev *evt);
int parse_message_json_obj(json_object *jmsg, struct tgev *evt);

#define TGEV_JSON_REPLY_TO ((void *)1)

static inline struct tgev *parse_replied_msg(json_object *jmsg)
{
	struct tgev *reply_to = malloc(sizeof(*reply_to));
	memset(reply_to, '\0', sizeof(*reply_to));

	if (unlikely(parse_message_json_obj(jmsg, reply_to) != 0)) {
		free(reply_to);
		return NULL;
	}
	reply_to->is_replied_node = true;
	reply_to->json = TGEV_JSON_REPLY_TO;

	return reply_to;
}


#define INCLUDE_SUB_TG_EVENT
#define SUB_TG_EVENT_CIRCULAR_INLINE
#include <gwbot/lib/tg_event/text.h>
#include <gwbot/lib/tg_event/photo.h>
#include <gwbot/lib/tg_event/sticker.h>
#include <gwbot/lib/tg_event/gif.h>
#undef SUB_TG_EVENT_CIRCULAR_INLINE
#undef INCLUDE_SUB_TG_EVENT


static inline struct tgev *tge_get_reply_to(struct tgev *evt)
{
	switch (evt->type) {
	case TGEV_UNKNOWN:
		break;
	case TGEV_TEXT:
		return evt->msg_text.reply_to;
	case TGEV_PHOTO:
		return evt->msg_text.reply_to;
	case TGEV_STICKER:
		return evt->msg_text.reply_to;
	case TGEV_GIF:
		return evt->msg_text.reply_to;
	}
	return NULL;
}



static inline int64_t tge_get_chat_id(struct tgev *evt)
{
	switch (evt->type) {
	case TGEV_UNKNOWN:
		break;
	case TGEV_TEXT:
		return evt->msg_text.chat.id;
	case TGEV_PHOTO:
		return evt->msg_photo.chat.id;
	case TGEV_STICKER:
		return evt->msg_sticker.chat.id;
	case TGEV_GIF:
		return evt->msg_gif.chat.id;
	}
	return -1;
}


static inline const char *tge_get_text(struct tgev *evt)
{
	switch (evt->type) {
	case TGEV_UNKNOWN:
		break;
	case TGEV_TEXT:
		return evt->msg_text.text;
	case TGEV_PHOTO:
		return evt->msg_photo.caption;
	case TGEV_STICKER:
		/* No text */
		break;
	case TGEV_GIF:
		return evt->msg_gif.caption;
	}
	return NULL;
}


static inline uint64_t tge_get_msg_id(struct tgev *evt)
{
	switch (evt->type) {
	case TGEV_UNKNOWN:
		break;
	case TGEV_TEXT:
		return evt->msg_text.msg_id;
	case TGEV_PHOTO:
		return evt->msg_photo.msg_id;
	case TGEV_STICKER:
		return evt->msg_sticker.msg_id;
	case TGEV_GIF:
		return evt->msg_gif.msg_id;
	}
	return 0;
}


static inline const struct tgevi_from *tge_get_from(struct tgev *evt)
{
	switch (evt->type) {
	case TGEV_UNKNOWN:
		break;
	case TGEV_TEXT:
		return &evt->msg_text.from;
	case TGEV_PHOTO:
		return &evt->msg_photo.from;
	case TGEV_STICKER:
		return &evt->msg_sticker.from;
	case TGEV_GIF:
		return &evt->msg_gif.from;
	}
	return NULL;
}


static inline uint64_t tge_get_user_id(struct tgev *evt)
{
	switch (evt->type) {
	case TGEV_UNKNOWN:
		break;
	case TGEV_TEXT:
		return evt->msg_text.from.id;
	case TGEV_PHOTO:
		return evt->msg_photo.from.id;
	case TGEV_STICKER:
		return evt->msg_sticker.from.id;
	case TGEV_GIF:
		return evt->msg_gif.from.id;
	}
	return 0;
}


#endif /* #ifndef GWBOT__LIB__TG_EVENT_H */
