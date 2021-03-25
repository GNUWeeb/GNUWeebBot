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
	struct tgevi_entity	*entities;
	struct tgev		*reply_to;
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



	if (unlikely(!json_object_object_get_ex(jmsg, "chat", &res))) {
		pr_err("Cannot find \"chat\" key on text event");
		return -EINVAL;
	}
	ret = parse_tgevi_chat(res, &etext->chat);
	if (unlikely(ret != 0))
		return ret;



	return ret;
}

#endif /* #ifdef SUB_TG_EVENT_CIRCULAR_INLINE */