// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_api.h
 *
 *  String helpers header for TeaVPN2
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__TG_API_H
#define GWBOT__LIB__TG_API_H

#include <gwbot/base.h>

#ifdef INCLUDE_SUB_TG_API
#  include <gwbot/lib/tg_api/send_message.h>
#endif


struct tg_api_res {
	char		*res;
	size_t		len;
};


#endif /* #ifndef GWBOT__LIB__TG_API_H */
