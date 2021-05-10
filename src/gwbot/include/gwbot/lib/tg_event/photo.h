// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event/photo.h
 *
 *  Photo message event header for events
 *
 *  Copyright (C) 2021  Aliwoto
 */


#ifndef GWBOT__LIB__TG_EVENT__PHOTO_H
#define GWBOT__LIB__TG_EVENT__PHOTO_H

#ifndef INCLUDE_SUB_TG_EVENT
#  error This header should only be included from <gwbot/lib/tg_event.h>
#endif



struct tgev_photo {
	uint64_t		msg_id;
	struct tgevi_from	from;
	struct tgevi_from	forward_from;
	const char 		*fwd_sender_name;
	struct tgevi_chat	chat;
	struct tgevi_chat	sender_chat;
	time_t			date;
	time_t			forward_date;
	bool			is_forwarded;
	bool			is_unknown_fwd;
	/* How many photo[n] there are? */
	uint16_t		photo_c;
	struct tgevi_file	*photo;

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
					       struct tgevi_file **photos)
{
	json_object *photo;
	struct tgevi_file *photos_tmp;

	photos_tmp = calloc(photo_c, sizeof(*photos_tmp));
	if (unlikely(photos_tmp == NULL)) {
		pr_err("calloc(): " PRERF, PREAR(ENOMEM));
		return -ENOMEM;
	}


	for (uint16_t i = 0; i < photo_c; i++) {
		struct tgevi_file *ph_ptr = &photos_tmp[i];
		photo = json_object_array_get_idx(jphotos, i);
		parse_tgevi_file(photo, ph_ptr);
	}

	*photos = photos_tmp;
	return 0;
}




static __always_inline int parse_event_photo(json_object *jphoto,
					     struct tgev *evt)
{
	int ret;
	json_object *res;
	struct tgev_photo *ephoto;


	if (unlikely(!json_object_object_get_ex(jphoto, "photo", &res))) {
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

	ephoto->photo_c = (uint16_t)json_object_array_length(res);
	if (unlikely(parse_tgevi_photos(res, ephoto->photo_c,
						 &ephoto->photo) < 0))
			return -EINVAL;


	if (unlikely(!json_object_object_get_ex(jphoto, "message_id", &res))) {
		pr_err("Cannot find \"message_id\" key on photo event");
		return -EINVAL;
	}
	ephoto->msg_id = json_object_get_uint64(res);


	if (unlikely(!json_object_object_get_ex(jphoto, "from", &res))) {
		pr_err("Cannot find \"from\" key on photo event");
		return -EINVAL;
	}
	ret = parse_tgevi_from(res, &ephoto->from);
	if (unlikely(ret != 0))
		return ret;


	if (likely(!json_object_object_get_ex(jphoto, "forward_from", &res))) {
		/*
		 * `forward_from` is not mandatory, but we should also check
		 * the `forward_sender_name` parameter for unknown forward.
		 */
		if (likely(!json_object_object_get_ex(jphoto,
						"forward_sender_name", &res)))
		{
			ephoto->forward_date	= 0;
			ephoto->is_forwarded 	= false;
			ephoto->is_unknown_fwd 	= false;
			ephoto->fwd_sender_name	= NULL;
		} else {
			ephoto->fwd_sender_name	= json_object_get_string(res);
			ephoto->is_forwarded	= true;
			ephoto->is_unknown_fwd	= true;
		}
	} else {
		ret = parse_tgevi_from(res, &ephoto->forward_from);
		if (unlikely(ret != 0))
			return ret;
		ephoto->is_forwarded	= true;
		ephoto->is_unknown_fwd	= false;
		ephoto->fwd_sender_name = NULL;
	}

	if (ephoto->is_forwarded)
	{
		if (unlikely(!json_object_object_get_ex(jphoto,
						"forward_date", &res))) {
			/*
			 * `forward_date` is originaly not mandatory,
		 	 * but since there is `forward_from` parameter,
		 	 * there SHOULD be `forward_date` parameter.
			 */
			pr_err("Cannot find \"forward_date\" key on text "
								"event");
			return -EINVAL;
		} else {
			ephoto->forward_date = (time_t)json_object_get_int64(res);
		}
	}

	if (unlikely(!json_object_object_get_ex(jphoto, "chat", &res))) {
		pr_err("Cannot find \"chat\" key on photo event");
		return -EINVAL;
	}
	ret = parse_tgevi_chat(res, &ephoto->chat);
	if (unlikely(ret != 0))
		return ret;

	if (unlikely(json_object_object_get_ex(jphoto, "sender_chat", &res))) {
		ret = parse_tgevi_chat(res, &ephoto->sender_chat);
		if (unlikely(ret != 0))
			return ret;
	}

	if (unlikely(!json_object_object_get_ex(jphoto, "date", &res))) {
		pr_err("Cannot find \"date\" key on photo event");
		return -EINVAL;
	}
	ephoto->date = (time_t)json_object_get_int64(res);

	if (unlikely(!json_object_object_get_ex(jphoto, "caption", &res))) {
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



	if (unlikely(!json_object_object_get_ex(jphoto, "caption_entities",
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
	if (!json_object_object_get_ex(jphoto, "reply_to_message", &res)) {
		/* `reply_to` is not mandatory */
		ephoto->reply_to = NULL;
	} else {
		/* TODO: Parse reply to */
		ephoto->reply_to = NULL;
	}

	return 0;
}



#endif /* #ifdef SUB_TG_EVENT_CIRCULAR_INLINE */
