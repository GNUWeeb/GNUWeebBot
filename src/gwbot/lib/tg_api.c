

#include <curl/curl.h>
#include <gwbot/lib/tg_api.h>


static bool global_init = false;

char *tg_bot_token;


static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *user)
{
	size_t realsize = size * nmemb;






	return realsize;
}


int tg_api_post(struct tg_api_res *res, const struct tg_api_req *req)
{
	CURL *curl;
	int ret = 0;
	CURLcode cres;
	char url[1024];


	if (unlikely(!global_init)) {
		curl_global_init(CURL_GLOBAL_ALL);
		global_init = true;
	}

	snprintf(url, sizeof(url), "https://api.telegram.org/bot%s/%s",
		 tg_bot_token, req->method);

	curl = curl_easy_init();
	if (unlikely(curl == NULL))
		return -ENOMEM;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->body);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, res);

	cres = curl_easy_perform(curl);
	if(unlikely(cres != CURLE_OK)) {
		pr_err("curl_easy_perform() failed: %s\n",
		       curl_easy_strerror(cres));
		ret = -1;
		goto out;
	}


out:
	if (likely(curl))
		curl_easy_cleanup(curl);
	return ret;
}
