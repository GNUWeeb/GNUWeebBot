
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

int parse_tga_admins(const char *res)
{

}

