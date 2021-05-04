

#include <stdlib.h>
#include <curl/curl.h>
#include <gwbot/lib/tg_api.h>


static bool global_init = false;


void tg_api_global_init(void)
{
	if (!global_init) {
		curl_global_init(CURL_GLOBAL_ALL);
		global_init = true;
	}
}


void tg_api_global_destroy(void)
{
	if (global_init) {
		curl_global_cleanup();
		global_init = false;
	}
}


static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp)
{
	char *body;
	tga_res_t *res;
	size_t len, allocated, add_len;

	res       = userp;
	body      = res->body;
	len       = res->len;
	allocated = res->allocated;
	add_len   = size * nmemb;

	if (unlikely((add_len + len + 2) >= allocated)) {
		/*
		 * We run out of space, resize it 2 time bigger.
		 */
		char *tmp;

		allocated  = (allocated * 2) + add_len;
		tmp        = realloc(body, allocated);
		if (unlikely(tmp == NULL))
			return 0;

		body           = tmp;
		res->body      = body;
		res->allocated = allocated;
	}


	memcpy(&body[len], data, add_len);

	len       += add_len;
	body[len]  = '\0';
	res->len   = len;
	return add_len;
}


static int tg_api_allocate_res(tga_res_t *res)
{
	if (res->body == NULL) {
		/*
		 * The `handle` doesn't hold an allocated heap pointer,
		 * so let's allocate a new one here.
		 */
		char *body;
		size_t allocated;

		allocated = 0x1000;
		body      = malloc(allocated);
		if (unlikely(body == NULL)) {
			memset(res, 0, sizeof(*res));
			pr_err("malloc(): " PRERF, PREAR(ENOMEM));
			return -ENOMEM;
		}

		res->body      = body;
		res->allocated = allocated;
		goto out;
	}


	/*
	 * Allow reuse the handle with pre-allocated heap
	 */
	if (unlikely(res->allocated == 0u)) {
		/*
		 * `res->allocated` must never be zero if
		 * `res->body` is not NULL
		 */
		pr_err("Bug: res->body is not NULL, but res->allocated "
		       "is zero");
		abort();
	}
out:
	res->len = 0u;
	return 0;
}


int tg_api_post(tga_handle_t *handle)
{
	int ret;
	CURL *curl;
	CURLcode cres;
	tga_res_t *res;
	tga_req_t *req;
	char url[0x1000];
	struct curl_slist *list = NULL;

	res = &handle->res;
	req = &handle->req;


	if (unlikely(req->method == NULL)) {
		pr_err("handle->method cannot be empty on tg_api_post");
		return -EINVAL;
	}


	if (unlikely(handle->token == NULL || *handle->token == '\0')) {
		pr_err("handle->token cannot be empty on tg_api_post");
		return -EINVAL;
	}


	ret = tg_api_allocate_res(res);
	if (unlikely(ret))
		return ret;


	snprintf(url, sizeof(url), "https://api.telegram.org/bot%s/%s",
		 handle->token, req->method);


	curl = curl_easy_init();
	if (unlikely(curl == NULL)) {
		pr_err("curl_easy_init(): " PRERF, PREAR(ENOMEM));
		return -ENOMEM;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->body);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, res);

	switch (handle->content_type) {
	case APP_X_WWW_FORM_URLENCODED:
		list = curl_slist_append(list, "Content-Type: application/x-www-form-urlencoded");
		break;
	case APP_JSON:
		list = curl_slist_append(list, "Content-Type: application/json");
		break;
	}

	if (list)
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

	prl_notice(7, "Curl to %s...", req->method);
	cres = curl_easy_perform(curl);
	if (unlikely(cres != CURLE_OK)) {
		pr_err("curl_easy_perform(): %s", curl_easy_strerror(cres));
		ret = -EBADMSG;
	}

	if (list != NULL) {
		curl_slist_free_all(list);
	}

	curl_easy_cleanup(curl);
	return ret;
}
