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

#include <stdlib.h>
#include <gwbot/base.h>
#include <gwbot/lib/string.h>


/*
 * See: https://core.telegram.org/bots/api#user
 */
struct tga_user {
	uint64_t	id;
	const char	*first_name;
	const char	*last_name;
	const char	*username;
	bool		is_bot;
	char		lang[4];
};


typedef enum _chmem_status {
	NOP_STATUS	= 0,
	CREATOR		= (1u << 1u),
	ADMINISTRATOR	= (1u << 2u),
	MEMBER		= (1u << 3u),
	RESTRICTED	= (1u << 4u),
	LEFT		= (1u << 5u),
	KICKED		= (1u << 6u),
} chmem_status_t;


/*
 * See: https://core.telegram.org/bots/api#chatmember
 */
struct tga_chat_member {
	struct tga_user		user;
	chmem_status_t		status;
};


typedef struct _tga_req_t {
	const char	*method;
	const char	*body;
} tga_req_t;


typedef struct _tga_res_t {
	char		*body;
	size_t		len;
	size_t		allocated;
} tga_res_t;


typedef enum _content_type_t {
	APP_X_WWW_FORM_URLENCODED	= 0,
	APP_JSON			= 1,
} content_type_t;


typedef struct _tga_handle_t {
	const char	*token;
	tga_req_t	req;
	tga_res_t	res;
	content_type_t	content_type;
} tga_handle_t;


typedef enum _parse_mode_t {
	PARSE_MODE_OFF		= 0,
	PARSE_MODE_HTML		= 1,
	PARSE_MODE_MARKDOWN	= 2
} parse_mode_t;


void tg_api_global_init(void);
void tg_api_global_destroy(void);

int tg_api_post(tga_handle_t *handle);


static __always_inline void tga_hdestroy(tga_handle_t *thandle)
{
	if (unlikely(thandle == NULL))
		return;

	free(thandle->res.body);
	memzero_explicit(thandle, sizeof(*thandle));
	free(thandle);
}


static __always_inline void tga_sdestroy(tga_handle_t *thandle)
{
	if (unlikely(thandle == NULL))
		return;

	free(thandle->res.body);
	memzero_explicit(thandle, sizeof(*thandle));
}


static __always_inline tga_handle_t *tga_screate(tga_handle_t *thandle,
						 const char *token)
{
	memset(thandle, 0, sizeof(*thandle));
	thandle->token = token;
	return thandle;
}


static __always_inline tga_handle_t *tga_hcreate(const char *token)
{
	tga_handle_t *thandle;

	thandle = malloc(sizeof(*thandle));
	if (unlikely(thandle == NULL)) {
		pr_err("malloc(): " PRERF, PREAR(ENOMEM));
		return NULL;
	}

	memset(thandle, 0, sizeof(*thandle));
	thandle->token = token;
	return thandle;
}


static __always_inline void tga_set_body(tga_handle_t *handle, const char *body)
{
	handle->req.body = body;
}


static __always_inline void tga_set_method(tga_handle_t *handle,
					   const char *method)
{
	handle->req.method = method;
}


static __always_inline const char *tge_get_res_body(tga_handle_t *handle)
{
	return handle->res.body;
}

#include <gwbot/lib/tg_api/kick_chat_member.h>
#include <gwbot/lib/tg_api/send_message.h>
#include <gwbot/lib/tg_api/unban_chat_member.h>
#include <gwbot/lib/tg_api/restrict_chat_member.h>
#include <gwbot/lib/tg_api/pin_chat_message.h>
#include <gwbot/lib/tg_api/unpin_chat_message.h>
#include <gwbot/lib/tg_api/get_chat_administrators.h>
#include <gwbot/lib/tg_api/delete_message.h>

#endif /* #ifndef GWBOT__LIB__TG_API_H */
