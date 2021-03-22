

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
	tg_api_res *res = userp;
	size_t len = res->len;
	size_t allocated = res->allocated;
	size_t add_len = size * nmemb;

	if (unlikely((add_len + len + 0x30) > allocated)) {
		char *tmp;

		allocated = (allocated * 2) + add_len;
		tmp = realloc(res->body, allocated);
		if (unlikely(tmp == NULL))
			return 0;

		res->body = tmp;
		res->allocated = allocated;
	}

	memcpy(&res->body[len], data, add_len);

	len += add_len;
	res->len = len;
	res->body[len] = '\0';

	return add_len;
}



int tg_api_post(tg_api_handle *handle)
{
	int ret = 0;
	CURLcode cres;
	tg_api_res *res;
	tg_api_req *req;
	CURL *curl = NULL;
	char url[1024];

	res = &handle->res;
	req = &handle->req;

	if (unlikely(req->method == NULL || handle->token == NULL))
		return -EINVAL;

	snprintf(url, sizeof(url), "https://api.telegram.org/bot%s/%s",
		 handle->token, req->method);


	/*
	 *
	 */


	res->len       = 0;
	res->allocated = 0x2000u;
	res->body      = malloc(res->allocated);
	if (unlikely(res->body == NULL)) {
		res->allocated = 0u;
		pr_err("malloc() failed: " PRERF, PREAR(ENOMEM));
		return -ENOMEM;
	}

	curl = curl_easy_init();
	if (unlikely(curl == NULL)) {
		pr_err("curl_easy_init() failed: " PRERF, PREAR(ENOMEM));
		ret = -ENOMEM;
		goto out;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->body);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, res);

	prl_notice(7, "Curl to %s...", req->method);
	cres = curl_easy_perform(curl);
	if (unlikely(cres != CURLE_OK)) {
		pr_err("curl_easy_perform() failed: %s\n",
		       curl_easy_strerror(cres));
		ret = -1;
		goto out;
	}
out:
	curl_easy_cleanup(curl);
	if (unlikely(ret != 0)) {
		free(res->body);
		res->body = NULL;
		res->allocated = 0u;
	}
	return ret;
}
