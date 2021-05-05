
#include <gwbot/base.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/pin_chat_message.h>


int tga_pin_chat_msg(tga_handle_t *handle, const tga_pin_cm_t *ctx)
{
	char buf[256];

	snprintf(buf, sizeof(buf),
		"chat_id=%" PRId64
		"&message_id=%" PRIu64
		"&disable_notification=%hhu",
		ctx->chat_id,
		ctx->message_id,
		ctx->disable_notification);

	tga_set_body(handle, buf);
	tga_set_method(handle, "pinChatMessage");
	return tg_api_post(handle);
}
