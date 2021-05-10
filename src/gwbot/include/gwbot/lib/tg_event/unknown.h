// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event/unknown.h
 *
 *  Unknown message event header for events
 *
 *  Copyright (C) 2021  Ammar Faizi
 */


#ifndef GWBOT__LIB__TG_EVENT__UNKNOWN_H
#define GWBOT__LIB__TG_EVENT__UNKNOWN_H

#ifndef INCLUDE_SUB_TG_EVENT
#  error This header should only be included from <gwbot/lib/tg_event.h>
#endif


struct tgev_unknown {
	uint64_t		msg_id;
	struct tgevi_from	from;
	struct tgevi_from	forward_from;
	const char		*fwd_sender_name;
	struct tgevi_chat	chat;
	struct tgevi_chat	sender_chat;
	time_t			date;
	time_t			forward_date;
	uint16_t		entity_c; /* How many entities[n] there are? */
	struct tgevi_entity	*entities;
	struct tgev		*reply_to;
	bool			is_forwarded;
	bool			is_unknown_fwd;
};

#endif /* #ifndef GWBOT__LIB__TG_EVENT__UNKNOWN_H */


#ifdef SUB_TG_EVENT_CIRCULAR_INLINE

static __always_inline int parse_event_unknown(json_object *jmsg,
					       struct tgev *evt)
{
	int ret;
	json_object *res;
	struct tgev_unknown *eukw;

	eukw = &evt->msg_unknown;
	evt->type = TGEV_UNKNOWN;

	if (unlikely(!json_object_object_get_ex(jmsg, "message_id", &res))) {
		pr_err("Cannot find \"message_id\" key on text event");
		return -EINVAL;
	}
	eukw->msg_id = json_object_get_uint64(res);


	if (unlikely(!json_object_object_get_ex(jmsg, "from", &res))) {
		pr_err("Cannot find \"from\" key on text event");
		return -EINVAL;
	}
	ret = parse_tgevi_from(res, &eukw->from);
	if (unlikely(ret != 0))
		return ret;


	if (likely(!json_object_object_get_ex(jmsg, "forward_from", &res))) {
		/*
		 * `forward_from` is not mandatory, but we should also check
		 * the `forward_sender_name` parameter for unknown forward.
		 */
		if (likely(!json_object_object_get_ex(jmsg,
						"forward_sender_name", &res))) {
			eukw->forward_date      = 0ul;
			eukw->is_forwarded 	= false;
			eukw->is_unknown_fwd 	= false;
			eukw->fwd_sender_name 	= NULL;
		} else {
			eukw->fwd_sender_name   = json_object_get_string(res);
			eukw->is_forwarded 	= true;
			eukw->is_unknown_fwd 	= true;
		}
	} else {
		ret = parse_tgevi_from(res, &eukw->forward_from);
		if (unlikely(ret != 0))
			return ret;
		eukw->is_forwarded	= true;
		eukw->is_unknown_fwd	= false;
		eukw->fwd_sender_name 	= NULL;
	}

	if (unlikely(eukw->is_forwarded)) {
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
			eukw->forward_date = (time_t)json_object_get_int64(res);
		}
	}

	if (unlikely(!json_object_object_get_ex(jmsg, "chat", &res))) {
		pr_err("Cannot find \"chat\" key on text event");
		return -EINVAL;
	}
	ret = parse_tgevi_chat(res, &eukw->chat);
	if (unlikely(ret != 0))
		return ret;

	if (unlikely(json_object_object_get_ex(jmsg, "sender_chat", &res))) {
		ret = parse_tgevi_chat(res, &eukw->sender_chat);
		if (unlikely(ret != 0))
			return ret;
	}

	if (unlikely(!json_object_object_get_ex(jmsg, "date", &res))) {
		pr_err("Cannot find \"date\" key on sticker event");
		return -EINVAL;
	}
	eukw->date = (time_t)json_object_get_int64(res);


	if (unlikely(!json_object_object_get_ex(jmsg, "entities", &res))) {
		/* `entities` is not mandatory */
		eukw->entity_c = 0u;
		eukw->entities = NULL;
	} else {
		uint16_t entity_c;

		entity_c = (uint16_t)json_object_array_length(res);
		if (likely(entity_c == 0u)) {
			eukw->entity_c = 0u;
			eukw->entities = NULL;
			goto parse_reply_to;
		}

		eukw->entity_c = entity_c;
		if (unlikely(parse_tgevi_entities(res, entity_c,
						 &eukw->entities) < 0))
			return -EINVAL;
	}


parse_reply_to:
	if (json_object_object_get_ex(jmsg, "reply_to_message", &res) && res) {
		eukw->reply_to = parse_replied_msg(res);
	} else {
		/* `reply_to` is not mandatory */
		eukw->reply_to = NULL;
	}

	return 0;
}

#endif /* #ifdef SUB_TG_EVENT_CIRCULAR_INLINE */
