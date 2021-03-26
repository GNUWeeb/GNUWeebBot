// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event/photo.h
 *
 *  Text message header for events
 *
 *  Copyright (C) 2021  aliwoto
 */

#ifndef GWBOT__LIB__TG_EVENT__PHOTO_H
#define GWBOT__LIB__TG_EVENT__PHOTO_H

#ifndef INCLUDE_SUB_TG_EVENT
#  error This header should only be included from <gwbot/lib/tg_event.h>
#endif

struct tgevi_photo_list {
	const char 		*file_id;
	const char		*file_unique_id;
	size_t			file_size;
	uint16_t		width;
	uint16_t		height;
};

struct tgev_photo {
	uint64_t		msg_id;
	struct tgevi_from	from;
	struct tgevi_chat	chat;
	time_t			date;

	/* How many photo[n] there are? */
	uint16_t		photo_c; 
	struct tgevi_photo_list	*photo;

	const char		*caption;

	/* How many caption_entities[n] there are? */
	uint16_t		caption_entity_c;
	struct tgevi_entity	*caption_entities;

	struct tgev		*reply_to;
};

#endif /* #ifndef GWBOT__LIB__TG_EVENT__PHOTO_H */



#ifdef SUB_TG_EVENT_CIRCULAR_INLINE

static __always_inline int parse_tgevi_photos(json_object *jphotos,
					       uint16_t photo_c,
					       struct tgevi_photo_list **photos)
{
	json_object *photo, *tmp;
	struct tgevi_photo_list *photos_tmp;

	photos_tmp = calloc(photo_c, sizeof(*photos_tmp));
	if (unlikely(photos_tmp == NULL)) {
		pr_err("calloc(): " PRERF, PREAR(ENOMEM));
		return -ENOMEM;
	}


	for (uint16_t i = 0; i < photo_c; i++) {
		struct tgevi_photo_list *ph_ptr = &photos_tmp[i];
		photo = json_object_array_get_idx(jphotos, i);

		if (!json_object_object_get_ex(photo, "file_id", &tmp))
			continue;
		ph_ptr->file_id = json_object_get_string(tmp);

		if (!json_object_object_get_ex(photo, "file_unique_id", &tmp))
			continue;
		ph_ptr->file_unique_id = json_object_get_string(tmp);

		if (!json_object_object_get_ex(photo, "file_size", &tmp))
			continue;
		ph_ptr->file_size = json_object_get_uint64(tmp);

		if (!json_object_object_get_ex(photo, "width", &tmp))
			continue;
		ph_ptr->width = (uint16_t)json_object_get_uint64(tmp);

		if (!json_object_object_get_ex(photo, "height", &tmp))
			continue;
		ph_ptr->height = (uint16_t)json_object_get_uint64(tmp);
	}

	*photos = photos_tmp;
	return 0;
}


static __always_inline int parse_event_photo(json_object *jmsg,
					     struct tgev *evt)
{
	int ret;
	json_object *res;
	struct tgev_photo *ephoto;


	if (unlikely(!json_object_object_get_ex(jmsg, "photo", &res))) {
		/*
		 * We don't find "photo" key, so it's not photo event.
		 *
		 * Return -ECANCELED, so the caller can continue to
		 * parse another type of event.
		 */
		return -ECANCELED;
	}
	ephoto = &evt->msg_photo;
	evt->type = TGEV_PHOTO;

	memset(ephoto, 0, sizeof(*ephoto));

        ephoto->photo_c = (uint16_t)json_object_array_length(res);
        if (unlikely(parse_tgevi_photos(res, ephoto->photo_c,
 						 &ephoto->photo) < 0))
 			return -EINVAL;


        if (unlikely(!json_object_object_get_ex(jmsg, "message_id", &res))) {
		pr_err("Cannot find \"message_id\" key on photo event");
		return -EINVAL;
	}
	ephoto->msg_id = json_object_get_uint64(res);


        if (unlikely(!json_object_object_get_ex(jmsg, "from", &res))) {
		pr_err("Cannot find \"from\" key on photo event");
		return -EINVAL;
	}
	ret = parse_tgevi_from(res, &ephoto->from);
	if (unlikely(ret != 0))
		return ret;



        if (unlikely(!json_object_object_get_ex(jmsg, "chat", &res))) {
		pr_err("Cannot find \"chat\" key on photo event");
		return -EINVAL;
	}
	ret = parse_tgevi_chat(res, &ephoto->chat);
	if (unlikely(ret != 0))
		return ret;


        if (unlikely(!json_object_object_get_ex(jmsg, "date", &res))) {
		pr_err("Cannot find \"date\" key on photo event");
		return -EINVAL;
	} else {
		ephoto->date = json_object_get_uint64(res);
	}


	if (unlikely(!json_object_object_get_ex(jmsg, "caption", &res))) {
		/*
		 * `caption` is not mandatory, and since there is 
		 *  no caption, there is no `caption_entities`
		 */
		ephoto->caption = NULL;
		ephoto->caption_entity_c = 0u;
		ephoto->caption_entities = NULL;
		goto parse_reply_to;
	} else {
		ephoto->caption = json_object_get_string(res);
	}



        if (unlikely(!json_object_object_get_ex(jmsg, "caption_entities",
        					&res))) {
		/* `entities` is not mandatory */
		ephoto->caption_entity_c = 0u;
		ephoto->caption_entities = NULL;
	} else {
		uint16_t entity_c;
		struct tgevi_entity **cp_ent;

 		entity_c = (uint16_t)json_object_array_length(res);
 		if (likely(entity_c == 0u)) {
 			ephoto->caption_entity_c = 0u;
			ephoto->caption_entities = NULL;
 			goto parse_reply_to;
 		}

 		cp_ent = &ephoto->caption_entities;
 		ephoto->caption_entity_c = entity_c;
 		if (unlikely(parse_tgevi_entities(res, entity_c, cp_ent) < 0))
 			return -EINVAL;
	}


parse_reply_to:
	if (!json_object_object_get_ex(jmsg, "reply_to_message", &res)) {
		/* `reply_to` is not mandatory */
		ephoto->reply_to = NULL;
	} else {
		/* TODO: Parse reply to */
		ephoto->reply_to = NULL;
	}

	return 0;
}

#endif /* #ifdef SUB_TG_EVENT_CIRCULAR_INLINE */
