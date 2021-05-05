
#include <json-c/json.h>
#include <gwbot/base.h>
#include <gwbot/lib/string.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/get_chat_administrators.h>


int tga_get_chat_admins(tga_handle_t *handle, int64_t chat_id)
{
	char buf[64];

	snprintf(buf, sizeof(buf), "chat_id=%" PRId64, chat_id);

	tga_set_body(handle, buf);
	tga_set_method(handle, "getChatAdministrators");
	return tg_api_post(handle);
}


static chmem_status_t map_to_status(const char *sta)
{
	if (sta == NULL)
		return NOP_STATUS;

	if (!strncmp("creator", sta, 7))
		return CREATOR;

	if (!strncmp("administrator", sta, 13))
		return ADMINISTRATOR;

	if (!strncmp("member", sta, 6))
		return MEMBER;

	if (!strncmp("restricted", sta, 10))
		return RESTRICTED;

	if (!strncmp("left", sta, 4))
		return CREATOR;

	if (!strncmp("kicked", sta, 6))
		return KICKED;

	return NOP_STATUS;
}


int parse_tga_admins(json_object *obj, struct tga_chat_member **admins,
		     size_t *admin_cp)
{
	size_t admin_c;
	json_object *res;
	struct tga_chat_member *admins_arr, *aptr;

	if (!json_object_object_get_ex(obj, "ok", &res) || !res) {
		pr_err("Cannot find \"ok\" key from JSON API on parse_tga_admins()");
		return -EINVAL;
	}

	if (!json_object_get_boolean(res)) {
		pr_err("Bad JSON on parse_tga_admins()");
		return -EBADMSG;
	}

	if (!json_object_object_get_ex(obj, "result", &res) || !res) {
		pr_err("Cannot find \"result\" key from JSON API on parse_tga_admins()");
		return -EINVAL;
	}

	admin_c = json_object_array_length(res);
	admins_arr = calloc(admin_c + 1, sizeof(*admins_arr));
	if (admins_arr == NULL) {
		pr_err("calloc(): Cannot allocate memory on parse_tga_admins()");
		return -ENOMEM;
	}

	for (size_t i = 0, j = 0; i < admin_c; i++) {
		json_object *subres_obj, *user_obj, *tmp;
		struct tga_user *user;

		aptr        = &admins_arr[j];
		user        = &aptr->user;
		subres_obj  = json_object_array_get_idx(res, i);

		if (subres_obj == NULL)
			continue;

		if (!json_object_object_get_ex(subres_obj, "user", &user_obj)
			|| !user_obj)
			continue;


		if (!json_object_object_get_ex(user_obj, "id", &tmp) || !tmp)
			continue;

		user->id = json_object_get_uint64(tmp);


		if (json_object_object_get_ex(user_obj, "is_bot", &tmp) && tmp)
			user->is_bot = json_object_get_boolean(tmp);

		if (json_object_object_get_ex(user_obj, "first_name", &tmp) && tmp)
			user->first_name = json_object_get_string(tmp);

		if (json_object_object_get_ex(user_obj, "username", &tmp) && tmp)
			user->username = json_object_get_string(tmp);

		if (json_object_object_get_ex(user_obj, "lang", &tmp) && tmp) {
			const char *lang = json_object_get_string(tmp);
			if (lang) {
				strncpy(user->lang, lang, 4);
				user->lang[3] = '\0';
			}
		}


		if (json_object_object_get_ex(subres_obj, "status", &tmp) && tmp) {
			const char *sta = json_object_get_string(tmp);
			aptr->status = map_to_status(sta);
		}

		j++;
	}

	*admins = admins_arr;
	if (admin_cp)
		*admin_cp = admin_c;
	return 0;
}
