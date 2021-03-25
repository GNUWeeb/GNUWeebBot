

#include <string.h>
#include <gwbot/base.h>
#include <gwbot/lib/tg_event.h>
#include <json-c/json_types.h>
#include <json-c/json_pointer.h>
#include <json-c/json_tokener.h>


static __always_inline int tg_event_json_parse(const char *json_str,
					       size_t length,
			       		       json_object **json_obj_p)
{
	int ret = 0;
	json_object *json_obj;
	struct json_tokener *tok;
	enum json_tokener_error jerr;

	tok = json_tokener_new();
	if (unlikely(tok == NULL))
		return -ENOMEM;

	do {
		json_obj = json_tokener_parse_ex(tok, json_str, length);
		jerr     = json_tokener_get_error(tok);
	} while (likely(jerr == json_tokener_continue));


	if (unlikely(jerr != json_tokener_success)) {
		ret = -1;
		pr_err("JSON parse: %s", json_tokener_error_desc(jerr));
	}

	*json_obj_p = json_obj;
	json_tokener_free(tok);
	return ret;
}


static int parse_update_id(json_object *json_obj, struct tgev *evt)
{
	json_object *res;

	if (unlikely(!json_object_object_get_ex(json_obj, "update_id", &res))) {
		pr_err("Cannot find key \"update_id\" from JSON");
		return -1;
	}
	evt->update_id = json_object_get_uint64(res);

	return 0;
}


static int parse_message(json_object *json_obj, struct tgev *evt)
{
	int ret;
	json_object *jmsg = NULL;

	if (unlikely(!json_object_object_get_ex(json_obj, "message", &jmsg))) {
		pr_err("Cannot find key \"message\" from JSON");
		return -1;
	}
	evt->json = json_obj;



	ret = parse_event_text(jmsg, evt);
	if (ret == 0 || ret != -ECANCELED)
		return ret;

	// ret = parse_event_photo(jmsg, evt);
	// if (ret == 0) {
	// 	/* Success */
	// 	evt->type = TGEV_PHOTO;
	// 	return 0;
	// }
	// if (ret != -ECANCELED)
	// 	return ret;

	pr_err("Unknown event from JSON");
	return ret;
}


static __no_inline int internal_tg_event_load(const char *json_str,
					      size_t length,
					      struct tgev *evt)
{
	int ret;
	json_object *json_obj = NULL;

	evt->json = NULL;
	ret = tg_event_json_parse(json_str, length, &json_obj);
	if (unlikely(ret) < 0)
		return ret;

	ret = parse_update_id(json_obj, evt);
	if (unlikely(ret != 0)) 
		goto out_err;

	ret = parse_message(json_obj, evt);
	if (unlikely(ret != 0)) 
		goto out_err;

	return 0;

out_err:
	json_object_put(json_obj);
	return ret;
}


/*
 *
 * If `json_str` length is known by the caller, it's more efficient
 * to pass the lenght argument directly, so we don't have to call
 * `strlen` to determine the `json_str` length.
 *
 */
int tg_event_load_len(const char *json_str, size_t length, struct tgev *evt)
{
	if (unlikely(json_str == NULL)) {
		pr_err("`json_str` argument cannot be NULL");
		return -EINVAL;
	}

	if (unlikely(evt == NULL)) {
		pr_err("`evt` argument cannot be NULL");
		return -EINVAL;
	}

	return internal_tg_event_load(json_str, length, evt);
}


int tg_event_load(const char *json_str, struct tgev *evt)
{
	size_t length;

	if (unlikely(json_str == NULL)) {
		pr_err("`json_str` argument cannot be NULL");
		return -EINVAL;
	}

	if (unlikely(evt == NULL)) {
		pr_err("`evt` argument cannot be NULL");
		return -EINVAL;
	}

	/*
	 *
	 * The caller doesn't give us the length information,
	 * so we need to call `strlen` to find the length.
	 *
	 */
	length = strlen(json_str) + 1; /* Include the '\0' */

	return internal_tg_event_load(json_str, length, evt);
}
