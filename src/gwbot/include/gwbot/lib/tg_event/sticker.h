// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event/sticker.h
 *
 *  Text sticker header for events
 *
 *  Copyright (C) 2021 - aliwoto
 */

#ifndef GWBOT__LIB__TG_EVENT__STICKER_H
#define GWBOT__LIB__TG_EVENT__STICKER_H

#ifndef INCLUDE_SUB_TG_EVENT
#  error This header should only be included from <gwbot/lib/tg_event.h>
#endif

struct tgevi_sticker {
	uint16_t		width;
	uint16_t		height;
	char			emoji[6];
	const char		*set_name;
	struct tgevi_file	thumb;
	bool			is_animated;
	const char 		*file_id;
	const char		*file_unique_id;
	size_t			file_size;
};


struct tgev_sticker {
	uint64_t		msg_id;
	struct tgevi_from	from;
	struct tgevi_from	forward_from;
	const			char *fwd_sender_name;
	struct tgevi_chat	chat;
	struct tgevi_chat	sender_chat;
	struct tgevi_sticker	sticker;
	time_t			date;
	time_t			forward_date;
	bool			is_forwarded;
	bool			is_unknown_fwd;
	struct tgev		*reply_to;
};
#endif /* #ifndef GWBOT__LIB__TG_EVENT__STICKER_H */


#ifdef SUB_TG_EVENT_CIRCULAR_INLINE

static __always_inline int parse_tgevi_sticker(json_object *jstk,
					       struct tgevi_sticker *istk)
{
	int ret;
	json_object *res;

	if (unlikely(!json_object_object_get_ex(jstk, "width", &res))) {
		pr_err("Cannot find \"width\" key on sticker event");
		return -EINVAL;
	}
	istk->width = (uint16_t)json_object_get_uint64(res);

	if (unlikely(!json_object_object_get_ex(jstk, "height", &res))) {
		pr_err("Cannot find \"height\" key on sticker event");
		return -EINVAL;
	}
	istk->height = (uint16_t)json_object_get_uint64(res);

	if (unlikely(!json_object_object_get_ex(jstk, "emoji", &res))) {
		pr_err("Cannot find \"emoji\" key on sticker event");
		return -EINVAL;
	}
	sane_strncpy(istk->emoji, json_object_get_string(res),
			     sizeof(istk->emoji));
	
	if (unlikely(!json_object_object_get_ex(jstk, "set_name", &res))) {
		pr_err("Cannot find \"set_name\" key on sticker event");
		return -EINVAL;
	}
	istk->set_name = json_object_get_string(res);
	
	if (unlikely(!json_object_object_get_ex(jstk, "is_animated", &res))) {
		pr_err("Cannot find \"is_animated\" key on sticker event");
		return -EINVAL;
	}
	istk->is_animated = json_object_get_boolean(res) ? true : false;

	if (unlikely(!json_object_object_get_ex(jstk, "file_id", &res))) {
		pr_err("Cannot find \"file_id\" key on sticker event");
		return -EINVAL;
	}
	istk->file_id = json_object_get_string(res);

	if (unlikely(!json_object_object_get_ex(jstk, "file_unique_id", 
						&res))) {
		pr_err("Cannot find \"file_unique_id\" key on sticker event");
		return -EINVAL;
	}
	istk->file_unique_id = json_object_get_string(res);

	if (unlikely(!json_object_object_get_ex(jstk, "file_size", &res))) {
		pr_err("Cannot find \"file_size\" key on sticker event");
		return -EINVAL;
	}
	istk->file_size = json_object_get_uint64(res);

	if (unlikely(!json_object_object_get_ex(jstk, "thumb", &res))) {
		pr_err("Cannot find \"thumb\" key on sticker event");
		return -EINVAL;
	}
	ret = parse_tgevi_file(res, &istk->thumb);
	if (unlikely(ret != 0))
		return ret;
	return 0;
}


static __always_inline int parse_event_sticker(json_object *jstk,
					     struct tgev *evt)
{
	int ret;
	json_object *res;
	struct tgev_sticker *estk;


	if (unlikely(!json_object_object_get_ex(jstk, "sticker", &res))) {
		/*
		 * We don't find "sticker" key, so it's not sticker event.
		 *
		 * Return -ECANCELED, so the caller can continue to
		 * parse another type of event.
		 */
		return -ECANCELED;
	}

	estk = &evt->msg_sticker;
	evt->type = TGEV_STICKER;

	ret = parse_tgevi_sticker(res, &estk->sticker);
	if (unlikely(ret != 0))
		return ret;
	



        if (unlikely(!json_object_object_get_ex(jstk, "message_id", &res))) {
		pr_err("Cannot find \"message_id\" key on sticker event");
		return -EINVAL;
	}
	estk->msg_id = json_object_get_uint64(res);


        if (unlikely(!json_object_object_get_ex(jstk, "from", &res))) {
		pr_err("Cannot find \"from\" key on sticker event");
		return -EINVAL;
	}
	ret = parse_tgevi_from(res, &estk->from);
	if (unlikely(ret != 0))
		return ret;


	if (likely(!json_object_object_get_ex(jstk, "forward_from", &res))) {
		/* 
		 * `forward_from` is not mandatory, but we should also check
		 * the `forward_sender_name` parameter for unknown forward.
		 */
		if (likely(!json_object_object_get_ex(jstk, 
						"forward_sender_name", &res)))
		{
			estk->forward_date	= 0;
			estk->is_forwarded	= false;
			estk->is_unknown_fwd	= false;
			estk->fwd_sender_name	= NULL;
		} else {
			estk->fwd_sender_name	= json_object_get_string(res);
			estk->is_forwarded	= true;
			estk->is_unknown_fwd	= true;
		}
	} else {
		ret = parse_tgevi_from(res, &estk->forward_from);
		if (unlikely(ret != 0))
			return ret;
		estk->is_forwarded	= true;
		estk->is_unknown_fwd	= false;
		estk->fwd_sender_name 	= NULL;

	}

	if (estk->is_forwarded)
	{
		if (unlikely(!json_object_object_get_ex(jstk, 
						"forward_date", &res))) {
			/*
			 * `forward_date` is originaly not mandatory, 
		 	 * but since there is `forward_from` parameter, 
		 	 * there SHOULD be `forward_date` parameter. 
			 */
			pr_err("Cannot find \"forward_date\" key on sticker "
							"event");
			return -EINVAL;
		} else {
			estk->forward_date = (time_t)json_object_get_int64(res);
		}
	}


        if (unlikely(!json_object_object_get_ex(jstk, "chat", &res))) {
		pr_err("Cannot find \"chat\" key on sticker event");
		return -EINVAL;
	}
	ret = parse_tgevi_chat(res, &estk->chat);
	if (unlikely(ret != 0))
		return ret;

	if (unlikely(json_object_object_get_ex(jstk, "sender_chat", &res))) {
		ret = parse_tgevi_chat(res, &estk->sender_chat);
		if (unlikely(ret != 0))
			return ret;
	}



        if (unlikely(!json_object_object_get_ex(jstk, "date", &res))) {
		pr_err("Cannot find \"date\" key on sticker event");
		return -EINVAL;
	}
	estk->date = (time_t)json_object_get_int64(res);

	if (!json_object_object_get_ex(jstk, "reply_to_message", &res)) {
		/* `reply_to` is not mandatory */
		estk->reply_to = NULL;
	} else {
		/* TODO: Parse reply to */
		estk->reply_to = NULL;
	}

	return 0;
}




#endif /* #ifdef SUB_TG_EVENT_CIRCULAR_INLINE */