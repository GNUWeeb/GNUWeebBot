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

#ifdef INCLUDE_SUB_TG_API
#  include <gwbot/lib/tg_api/send_message.h>
#endif

/* 
 * Request.
 */
typedef struct _tg_api_req {
	const char	*method;
	const char	*body;
} tg_api_req;

/*
 * Response.
 */
typedef struct _tg_api_res {
	char		*body;
	size_t		len;
	size_t		allocated;
} tg_api_res;


/*
 * Telegram API response.
 */
typedef struct _tg_api_handle {
	const char	*token;
	tg_api_req	req;
	tg_api_res	res;
} tg_api_handle;


void tg_api_global_init(void);
void tg_api_global_destroy(void);
int tg_api_post(tg_api_handle *handle);


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


#endif /* #ifndef GWBOT__LIB__TG_API_H */
