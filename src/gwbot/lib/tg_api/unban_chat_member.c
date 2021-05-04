
#include <gwbot/base.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/unban_chat_member.h>


int tga_unban_chat_member(tga_handle_t *handle, const tga_unban_cm_t *ctx)
{
	char buf[256];

	snprintf(buf, sizeof(buf),
		 "chat_id=%" PRId64
		 "&user_id=%" PRIu64
		 "&only_if_banned=%hhu",
		 ctx->chat_id,
		 ctx->user_id,
		 ctx->only_if_banned);

	tga_set_body(handle, buf);
	tga_set_method(handle, "unbanChatMember");
	return tg_api_post(handle);
}
