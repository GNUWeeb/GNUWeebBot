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

#ifdef INCLUDE_SUB_TG_API
#  include <gwbot/lib/tg_api/send_message.h>
#  include <gwbot/lib/tg_api/send_message.h>
#endif

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
	CREATOR		= (1 << 0),
	ADMINISTRATOR	= (1 << 1),
	MEMBER		= (1 << 2),
	RESTRICTED	= (1 << 3),
	LEFT		= (1 << 4),
	KICKED		= (1 << 5),
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


typedef struct _tga_handle_t {
	const char	*token;
	tga_req_t	req;
	tga_res_t	res;
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


#define qwe 0
#if qwe
/*
 * Handle create
 */
static inline void tg_api_hcreate(tga_handle_t *handle, const char *token)
{

}


/*
 * Set request
 */
static inline void tg_api_req_set_body(tg_api_req *req, const char *body)
{
	req->body = body;
}

static inline void tg_api_req_set_method(tg_api_req *req, const char *method)
{
	req->method = method;
}


/*
 * Get response
 */
static inline char *tg_api_res_get_body(tg_api_res *res)
{
	return res->body;
}

static inline size_t tg_api_res_get_len(tg_api_res *res)
{
	return res->len;
}

static inline size_t tg_api_res_get_alloc(tg_api_res *res)
{
	return res->allocated;
}


/*
 * tg_api_*
 */
static inline tg_api_handle *tg_api_hcreate(const char *token)
{
	tg_api_handle *handle;

	handle = malloc(sizeof(*handle));
	if (unlikely(handle == NULL)) {
		pr_err("malloc failed: " PRERF, PREAR(ENOMEM));
		return NULL;
	}
	memset(handle, 0, sizeof(*handle));

	handle->token = token;
	return handle;
}

/*
 * Set request
 */
static inline void tg_api_set_method(tg_api_handle *handle, const char *method)
{
	tg_api_req_set_method(&handle->req, method);
}

static inline void tg_api_set_body(tg_api_handle *handle, const char *body)
{
	tg_api_req_set_body(&handle->req, body);
}


/*
 * Get response
 */
static inline char *tg_api_get_body(tg_api_handle *handle)
{
	return tg_api_res_get_body(&handle->res);
}

static inline size_t tg_api_get_len(tg_api_handle *handle)
{
	return tg_api_res_get_len(&handle->res);
}

static inline size_t tg_api_get_alloc(tg_api_handle *handle)
{
	return tg_api_res_get_alloc(&handle->res);
}


static inline void tg_api_destroy(tg_api_handle *handle)
{
	tg_api_res *res = &handle->res;
	free(res->body);
	res->len = 0u;
	res->body = NULL;
	res->allocated = 0u;
	free(handle);
}
#endif

#endif /* #ifndef GWBOT__LIB__TG_API_H */
