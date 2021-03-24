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
	char			*text;
	struct tgevi_entity	*entities;
	struct tgev		*reply_to;
};

#endif /* #ifndef GWBOT__LIB__TG_EVENT__TEXT_H */
