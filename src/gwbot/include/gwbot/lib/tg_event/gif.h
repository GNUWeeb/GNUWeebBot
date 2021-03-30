// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event/gif.h
 *
 *  gif message header for events
 *
 *  Copyright (C) 2021 - aliwoto
 */


#ifndef GWBOT__LIB__TG_EVENT__GIF_H
#define GWBOT__LIB__TG_EVENT__GIF_H

#ifndef INCLUDE_SUB_TG_EVENT
#  error This header should only be included from <gwbot/lib/tg_event.h>
#endif


struct tgev_gif {
	uint64_t		msg_id;
	struct tgevi_from	from;
	struct tgevi_from	forward_from;
	const			char *fwd_sender_name;
	struct tgevi_chat	chat;
	struct tgevi_chat	sender_chat;
	struct tgevi_media	animation;
	struct tgevi_file	document;
	time_t			date;
	time_t			forward_date;
	bool			is_forwarded;
	bool			is_unknown_fwd;

	const char		*caption;
	/* How many caption_entities[n] there are? */
	uint16_t		caption_entity_c;
	struct tgevi_entity	*caption_entities;

	struct tgev		*reply_to;
};

#endif /* #ifdef GWBOT__LIB__TG_EVENT__GIF_H */

#ifdef SUB_TG_EVENT_CIRCULAR_INLINE


static __always_inline int parse_event_gif(json_object *jgif,
					     struct tgev *evt)
{
	int ret;
	json_object *res;
	struct tgev_gif *egif;


	if (unlikely(!json_object_object_get_ex(jgif, "animation", &res))) {
		/*
		 * We don't find "animation" key, so it's not gif event.
		 *
		 * Return -ECANCELED, so the caller can continue to
		 * parse another type of event.
		 */
		return -ECANCELED;
	}

	egif = &evt->msg_gif;
	evt->type = TGEV_GIF;
	ret = parse_tgevi_media(res, &egif->animation);
	if (unlikely(ret != 0))
		return ret;
	
	if (unlikely(!json_object_object_get_ex(jgif, "document", &res))) {
		pr_err("Cannot find \"document\" key on gif event");
		return -EINVAL;
	}
	ret = parse_tgevi_file(res, &egif->document);

        if (unlikely(!json_object_object_get_ex(jgif, "message_id", &res))) {
		pr_err("Cannot find \"message_id\" key on gif event");
		return -EINVAL;
	}
	egif->msg_id = json_object_get_uint64(res);


        if (unlikely(!json_object_object_get_ex(jgif, "from", &res))) {
		pr_err("Cannot find \"from\" key on gif event");
		return -EINVAL;
	}
	ret = parse_tgevi_from(res, &egif->from);
	if (unlikely(ret != 0))
		return ret;


	if (likely(!json_object_object_get_ex(jgif, "forward_from", &res))) {
		/* 
		 * `forward_from` is not mandatory, but we should also check
		 * the `forward_sender_name` parameter for unknown forward.
		 */
		if (likely(!json_object_object_get_ex(jgif, 
						"forward_sender_name", &res)))
		{
			egif->forward_date	= 0;
			egif->is_forwarded	= false;
			egif->is_unknown_fwd	= false;
			egif->fwd_sender_name	= NULL;
		} else {
			egif->fwd_sender_name	= json_object_get_string(res);
			egif->is_forwarded	= true;
			egif->is_unknown_fwd	= true;
		}
	} else {
		ret = parse_tgevi_from(res, &egif->forward_from);
		if (unlikely(ret != 0))
			return ret;
		egif->is_forwarded	= true;
		egif->is_unknown_fwd	= false;
		egif->fwd_sender_name 	= NULL;

	}

	if (egif->is_forwarded)
	{
		if (unlikely(!json_object_object_get_ex(jgif, 
						"forward_date", &res))) {
			/*
			 * `forward_date` is originaly not mandatory, 
		 	 * but since there is `forward_from` parameter, 
		 	 * there SHOULD be `forward_date` parameter. 
			 */
			pr_err("Cannot find \"forward_date\" key on gif "
							"event");
			return -EINVAL;
		} else {
			egif->forward_date = 
				(time_t)json_object_get_int64(res);
		}
	}


        if (unlikely(!json_object_object_get_ex(jgif, "chat", &res))) {
		pr_err("Cannot find \"chat\" key on gif event");
		return -EINVAL;
	}
	ret = parse_tgevi_chat(res, &egif->chat);
	if (unlikely(ret != 0))
		return ret;

	if (unlikely(json_object_object_get_ex(jgif, "sender_chat", &res))) {
		ret = parse_tgevi_chat(res, &egif->sender_chat);
		if (unlikely(ret != 0))
			return ret;
	}

        if (unlikely(!json_object_object_get_ex(jgif, "date", &res))) {
		pr_err("Cannot find \"date\" key on gif event");
		return -EINVAL;
	}
	egif->date = (time_t)json_object_get_int64(res);

	if (unlikely(!json_object_object_get_ex(jgif, "caption", &res))) {
		/*
		 * `caption` is not mandatory, and since there is 
		 *  no caption, there is no `caption_entities`
		 */
		egif->caption = NULL;
		egif->caption_entity_c = 0u;
		egif->caption_entities = NULL;
		goto parse_reply_to;
	} else {
		egif->caption = json_object_get_string(res);
	}



        if (unlikely(!json_object_object_get_ex(jgif, "caption_entities",
        					&res))) {
		/* `entities` is not mandatory */
		egif->caption_entity_c = 0u;
		egif->caption_entities = NULL;
	} else {
		uint16_t entity_c;
		struct tgevi_entity **cp_ent;

 		entity_c = (uint16_t)json_object_array_length(res);
 		if (likely(entity_c == 0u)) {
 			egif->caption_entity_c = 0u;
			egif->caption_entities = NULL;
 			goto parse_reply_to;
 		}

 		cp_ent = &egif->caption_entities;
 		egif->caption_entity_c = entity_c;
		ret = parse_tgevi_entities(res, entity_c, cp_ent);
 		if (unlikely(ret < 0))
 			return ret;
	}


parse_reply_to:
	if (!json_object_object_get_ex(jgif, "reply_to_message", &res)) {
		/* `reply_to` is not mandatory */
		egif->reply_to = NULL;
	} else {
		/* TODO: Parse reply to */
		egif->reply_to = NULL;
	}

	return 0;
}


#endif /* #ifdef SUB_TG_EVENT_CIRCULAR_INLINE */
