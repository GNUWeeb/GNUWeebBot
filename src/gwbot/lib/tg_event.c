

#include <string.h>
#include <gwbot/base.h>
#include <gwbot/lib/tg_event.h>
#include <json-c/json.h>


static __always_inline int tg_event_json_parse(const char *json_str,
					       size_t length,
			       		       json_object **json_obj_p)
{
	int ret = 0;
	json_object *json_obj = NULL;
	struct json_tokener *tok;
	enum json_tokener_error jerr;

	tok = json_tokener_new();
	if (unlikely(tok == NULL))
		return -ENOMEM;

	do {
		json_obj = json_tokener_parse_ex(tok, json_str, (int)length);
		jerr     = json_tokener_get_error(tok);
	} while (likely(jerr == json_tokener_continue));


	if (unlikely(jerr != json_tokener_success)) {
		ret = -EINVAL;
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
		return -EINVAL;
	}
	evt->update_id = json_object_get_uint64(res);

	return 0;
}

int parse_message_json_obj(json_object *jmsg, struct tgev *evt)
{
	int ret;

	ret = parse_event_text(jmsg, evt);
	if (ret == 0 || ret != -ECANCELED)
		return ret;

	ret = parse_event_photo(jmsg, evt);
	if (ret == 0 || ret != -ECANCELED)
		return ret;

	ret = parse_event_sticker(jmsg, evt);
	if (ret == 0 || ret != -ECANCELED)
		return ret;

	ret = parse_event_gif(jmsg, evt);
	if (ret == 0 || ret != -ECANCELED)
		return ret;

	evt->type = TGEV_UNKNOWN;
	return ret;
}


int parse_message(json_object *json_obj, struct tgev *evt)
{
	int ret;
	json_object *jmsg = NULL;

	if (unlikely(!json_object_object_get_ex(json_obj, "message", &jmsg))) {
		pr_err("Cannot find key \"message\" from JSON");
		return -EINVAL;
	}

	evt->is_replied_node = false;

	ret = parse_message_json_obj(jmsg, evt);
	if (evt->type == TGEV_UNKNOWN)
		pr_err("Unknown event from JSON");

	return ret;
}


static int internal_tg_event_load_str(const char *json_str, size_t length,
				      struct tgev *evt)
{
	int ret;

	ret = tg_event_json_parse(json_str, length, &evt->json);
	if (unlikely(ret) < 0)
		return ret;

	ret = parse_update_id(evt->json, evt);
	if (unlikely(ret != 0)) 
		goto out_err;

	ret = parse_message(evt->json, evt);
	if (unlikely(ret != 0)) 
		goto out_err;

	return 0;

out_err:
	tg_event_destroy(evt);
	return ret;
}


/*
 *
 * If `json_str` length is known by the caller, it's more efficient
 * to pass the lenght argument directly, so we don't have to call
 * `strlen` to determine the `json_str` length.
 *
 */
int tg_event_load_str_len(const char *json_str, size_t length, struct tgev *evt)
{
	if (unlikely(json_str == NULL)) {
		pr_err("`json_str` argument cannot be NULL");
		return -EINVAL;
	}

	if (unlikely(evt == NULL)) {
		pr_err("`evt` argument cannot be NULL");
		return -EINVAL;
	}

	return internal_tg_event_load_str(json_str, length, evt);
}


int tg_event_load_str(const char *json_str, struct tgev *evt)
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

	return internal_tg_event_load_str(json_str, length, evt);
}


static void destroy_reply_to_msg(struct tgev *evt)
{
	tg_event_destroy(evt);
	free(evt);	
}


static void tg_event_destroy_text(struct tgev_text *etext)
{
	if (unlikely(etext->entity_c > 0))
		free(etext->entities);

	if (etext->reply_to)
		destroy_reply_to_msg(etext->reply_to);
}

static void tg_event_destroy_photo(struct tgev_photo *ephoto)
{
	free(ephoto->photo);
	if (unlikely(ephoto->caption_entity_c > 0))
		free(ephoto->caption_entities);

	if (ephoto->reply_to)
		destroy_reply_to_msg(ephoto->reply_to);
}

static void tg_event_destroy_gif(struct tgev_gif *egif)
{
	if (unlikely(egif->caption_entity_c > 0))
		free(egif->caption_entities);

	if (egif->reply_to)
		destroy_reply_to_msg(egif->reply_to);
}


static void tg_event_destroy_sticker(struct tgev_sticker *esticker)
{
	if (esticker->reply_to)
		destroy_reply_to_msg(esticker->reply_to);
}


void tg_event_destroy(struct tgev *evt)
{
	int ret;

	switch (evt->type) {
	case TGEV_UNKNOWN:
		break;
	case TGEV_TEXT:
		tg_event_destroy_text(&evt->msg_text);
		break;
	case TGEV_PHOTO:
		tg_event_destroy_photo(&evt->msg_photo);
		break;
	case TGEV_STICKER:
		tg_event_destroy_sticker(&evt->msg_sticker);
		break;
	case TGEV_GIF:
		tg_event_destroy_gif(&evt->msg_gif);
		break;
	}

	if (evt->is_replied_node)
		return;

	if (unlikely(evt->json == NULL || evt->json == TGEV_JSON_REPLY_TO))
		return;


	ret = json_object_put(evt->json);
	evt->json = NULL;
	if (unlikely(ret != 1)) {
		panic("Invalid tg_event_destroy, object has more than 1 "
		      "reference (ret: %d)", ret);
		exit(1);
	}
}
