
#include <gwbot/base.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/unpin_chat_message.h>


int tga_unpin_chat_msg(tga_handle_t *handle, const tga_unpin_cm_t *ctx)
{
	char buf[128];

	snprintf(buf, sizeof(buf),
		"chat_id=%" PRId64
		"&message_id=%" PRIu64,
		ctx->chat_id,
		ctx->message_id);

	tga_set_body(handle, buf);
	tga_set_method(handle, "unpinChatMessage");
	return tg_api_post(handle);
}
