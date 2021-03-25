// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event/text.h
 *
 *  Text message header for events
 *
 *  Copyright (C) 2021  aliwoto
 */

#ifndef GWBOT__LIB__TG_EVENT__TEXT_H
#define GWBOT__LIB__TG_EVENT__TEXT_H

#ifndef INCLUDE_SUB_TG_EVENT
#  error This header should only be included from <gwbot/lib/tg_event.h>
#endif

struct tgev_text {
	uint64_t		msg_id;
	struct tgevi_from	from;
	struct tgevi_chat	chat;
	time_t			date;
	const char		*text;
	uint16_t		entity_c; /* How many entities[n] there are? */
	struct tgevi_entity	*entities;
	struct tgev		*reply_to;
};


#endif /* #ifndef GWBOT__LIB__TG_EVENT__TEXT_H */


#ifdef SUB_TG_EVENT_CIRCULAR_INLINE


static __always_inline int parse_text_entities(json_object *jentities,
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



	if (unlikely(!json_object_object_get_ex(jmsg, "chat", &res))) {
		pr_err("Cannot find \"chat\" key on text event");
		return -EINVAL;
	}
	ret = parse_tgevi_chat(res, &etext->chat);
	if (unlikely(ret != 0))
		return ret;



	if (unlikely(!json_object_object_get_ex(jmsg, "date", &res))) {
		/* `date` is not mandatory */
		etext->date = 0ul;
	} else {
		etext->date = json_object_get_uint64(res);
	}



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
 		if (unlikely(parse_text_entities(res, entity_c,
 						 &etext->entities) < 0))
 			return -EINVAL;
	}


parse_reply_to:
	if (!json_object_object_get_ex(jmsg, "reply_to_message_id", &res)) {
		/* `reply_to` is not mandatory */
		etext->reply_to = NULL;
	} else {
		/* TODO: Parse reply to */
		etext->reply_to = NULL;
	}

	return 0;
}

#endif /* #ifdef SUB_TG_EVENT_CIRCULAR_INLINE */
