
#include <json_types.h>
#include <json_tokener.h>

#include <gwbot/base.h>
#include <gwbot/lib/tg_event.h>




static json_object *tg_event_json_parse(const char *json_str)
{
	size_t length;
	json_object *json_obj = NULL;
	struct json_tokener tok;
	enum json_tokener_error jerr;

	json_tokener_reset(&tok);
	length = strlen(json_str) + 1; // Include '\0'
	do {
		json_obj = json_tokener_parse_ex(&tok, json_str, length);
		jerr     = json_tokener_get_error(&tok);

		if (likely(jerr != json_tokener_continue))
			break;

	} while (true);


	if (unlikely(jerr != json_tokener_success)) {
		pr_err("JSON parse: %s", json_tokener_error_desc(jerr));
		return NULL;
	}

	return json_obj;
}


int tg_event_load(const char *json_str, struct tgev *evt)
{
	json_object *json_obj;

	if (unlikely(json_str == NULL)) {
		pr_err("`json_str` argument cannot be NULL");
		return -EINVAL;
	}

	if (unlikely(evt == NULL)) {
		pr_err("`evt` argument cannot be NULL");
		return -EINVAL;
	}

	json_obj = tg_event_json_parse(json_str);
	if (unlikely(evt == NULL))
		return -1;
}
