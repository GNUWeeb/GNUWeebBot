// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event/text.h
 *
 *  Text message event header for events
 *
 *  Copyright (C) 2021  Aliwoto
 *  Copyright (C) 2021  Ammar Faizi
 */


#ifndef GWBOT__LIB__TG_EVENT__TEXT_H
#define GWBOT__LIB__TG_EVENT__TEXT_H

#ifndef INCLUDE_SUB_TG_EVENT
#  error This header should only be included from <gwbot/lib/tg_event.h>
#endif

struct tgev_text {
	uint64_t		msg_id;
	struct tgevi_from	from;
	struct tgevi_from	forward_from;
	const char		*fwd_sender_name;
	struct tgevi_chat	chat;
	struct tgevi_chat	sender_chat;
	time_t			date;
	time_t			forward_date;
	const char		*text;
	uint16_t		entity_c; /* How many entities[n] there are? */
	struct tgevi_entity	*entities;
	struct tgev		*reply_to;
	bool			is_forwarded;
	bool			is_unknown_fwd;
};


#endif /* #ifndef GWBOT__LIB__TG_EVENT__TEXT_H */


#ifdef SUB_TG_EVENT_CIRCULAR_INLINE


static __always_inline int parse_event_text(json_object *jmsg, struct tgev *evt)
{
	int ret;
	json_object *res;
	struct tgev_text *etext;


	if (unlikely(!json_object_object_get_ex(jmsg, "text", &res))) {
		/*
		 * We don't find "text" key, so it's not text event.
		 *
		 * Return -ECANCELED, so the caller can continue to
		 * parse another type of event.
		 */
		return -ECANCELED;
	}
	etext = &evt->msg_text;
	evt->type = TGEV_TEXT;
	etext->text = json_object_get_string(res);



	if (unlikely(!json_object_object_get_ex(jmsg, "message_id", &res))) {
		pr_err("Cannot find \"message_id\" key on text event");
		return -EINVAL;
	}
	etext->msg_id = json_object_get_uint64(res);



	if (unlikely(!json_object_object_get_ex(jmsg, "from", &res))) {
		pr_err("Cannot find \"from\" key on text event");
		return -EINVAL;
	}
	ret = parse_tgevi_from(res, &etext->from);
	if (unlikely(ret != 0))
		return ret;


	if (likely(!json_object_object_get_ex(jmsg, "forward_from", &res))) {
		/* 
		 * `forward_from` is not mandatory, but we should also check
		 * the `forward_sender_name` parameter for unknown forward.
		 */
		if (likely(!json_object_object_get_ex(jmsg, 
						"forward_sender_name", &res))) {
			etext->forward_date     = 0ul;
			etext->is_forwarded 	= false;
			etext->is_unknown_fwd 	= false;
			etext->fwd_sender_name 	= NULL;
		} else {
			etext->fwd_sender_name  = json_object_get_string(res);
			etext->is_forwarded 	= true;
			etext->is_unknown_fwd 	= true;
		}
	} else {
		ret = parse_tgevi_from(res, &etext->forward_from);
		if (unlikely(ret != 0))
			return ret;
		etext->is_forwarded	= true;
		etext->is_unknown_fwd	= false;
		etext->fwd_sender_name 	= NULL;
	}
	
	if (unlikely(etext->is_forwarded)) {
		if (unlikely(!json_object_object_get_ex(jmsg, 
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
			etext->forward_date = (time_t)json_object_get_int64(res);
		}
	}

	if (unlikely(!json_object_object_get_ex(jmsg, "chat", &res))) {
		pr_err("Cannot find \"chat\" key on text event");
		return -EINVAL;
	}
	ret = parse_tgevi_chat(res, &etext->chat);
	if (unlikely(ret != 0))
		return ret;

	if (unlikely(json_object_object_get_ex(jmsg, "sender_chat", &res))) {
		ret = parse_tgevi_chat(res, &etext->sender_chat);
		if (unlikely(ret != 0))
			return ret;
	}

	if (unlikely(!json_object_object_get_ex(jmsg, "date", &res))) {
		pr_err("Cannot find \"date\" key on sticker event");
		return -EINVAL;
	}
	etext->date = (time_t)json_object_get_int64(res);


	if (unlikely(!json_object_object_get_ex(jmsg, "entities", &res))) {
		/* `entities` is not mandatory */
		etext->entity_c = 0u;
		etext->entities = NULL;
	} else {
		uint16_t entity_c;

 		entity_c = (uint16_t)json_object_array_length(res);
 		if (likely(entity_c == 0u)) {
 			etext->entity_c = 0u;
			etext->entities = NULL;
 			goto parse_reply_to;
 		}

 		etext->entity_c = entity_c;
 		if (unlikely(parse_tgevi_entities(res, entity_c,
 						 &etext->entities) < 0))
 			return -EINVAL;
	}


parse_reply_to:
	if (json_object_object_get_ex(jmsg, "reply_to_message", &res) && res) {
		etext->reply_to = parse_replied_msg(res);
	} else {
		/* `reply_to` is not mandatory */
		etext->reply_to = NULL;
	}

	return 0;
}

#endif /* #ifdef SUB_TG_EVENT_CIRCULAR_INLINE */
