
#include <gwbot/base.h>
#include <gwbot/lib/string.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/kick_chat_member.h>


int tga_kick_chat_member(tga_handle_t *handle, const tga_kick_cm_t *ctx)
{
	char buf[512];
	size_t pos = 0;
	const size_t space = sizeof(buf);

	pos += (size_t)snprintf(buf, space,
				"chat_id=%" PRId64 "&user_id=%" PRIu64,
				ctx->chat_id,
				ctx->user_id);

	if (ctx->until_date)
		pos += (size_t)snprintf(buf + pos, space - pos,
					"&until_date=%" PRIu64,
					(uint64_t)ctx->until_date);

	if (!ctx->revoke_msg)
		memcpy(buf + pos, "&revoke_messages=0\0", 19);

	tga_set_body(handle, buf);
	tga_set_method(handle, "kickChatMember");
	return tg_api_post(handle);
}
