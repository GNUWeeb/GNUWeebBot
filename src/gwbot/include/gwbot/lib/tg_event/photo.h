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
	uint32_t		width;
	uint32_t		height;
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
