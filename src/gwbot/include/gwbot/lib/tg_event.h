// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_event.h
 *
 *  String helpers header for TeaVPN2
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__TG_EVENT_H
#define GWBOT__LIB__TG_EVENT_H

#include <time.h>
#include <stdlib.h>
#include <gwbot/base.h>


/*
 *
 * `tgev` means `tg event` (Telegram Event)
 * `tgevi` means `tg event info` (Telegram Event Info)
 *
 */


struct tgevi_from {
	uint64_t	id;
	char		*first_name;
	char		*last_name;
	char		*username;
	char		lang[4];
	bool		is_bot;
	struct_pad(0, 3);
};


struct tgevi_chat {
	int64_t		id;
	char		*title;
	char		*type;
	char		*username;
};


struct tgevi_entity {
	uint16_t	offset;
	uint16_t	length;
	char		*type;
};


typedef enum _tgev_type_t {
	TGEV_TEXT	= 0,
	TGEV_PHOTO	= 1,
	TGEV_STICKER	= 2,
} tgev_type_t;


#define INCLUDE_SUB_TG_EVENT
#include <gwbot/lib/tg_event/text.h>
#include <gwbot/lib/tg_event/photo.h>
#include <gwbot/lib/tg_event/sticker.h>
#undef INCLUDE_SUB_TG_EVENT

struct tgev {
	uint64_t	update_id;
	tgev_type_t	type;
	union {
		struct tgev_text	text;
		struct tgev_photo	photo;
		struct tgev_sticker	sticker;
	};
};


#endif /* #ifndef GWBOT__LIB__TG_EVENT_H */
