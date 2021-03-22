

#include <gwbot/base.h>
#include <gwbot/lib/string.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/send_message.h>


int tga_send_msg(tg_api_handle *handle, tg_api_smsg *ctx)
{
	size_t pos = 0;
	char buf[8096];
	const size_t space = sizeof(buf);

	pos = (size_t)snprintf(buf, space, "chat_id=" PRId64, ctx->chat_id);
	if (ctx->reply_to_msg_id) {
		pos += (size_t)snprintf(buf + pos, space - pos,
					"&reply_to_message_id=" PRIu64,
					ctx->reply_to_msg_id);
	}

	if (ctx->parse_mode) {
		strcpy(buf + pos, "&parse_mode=");
		pos += sizeof("&parse_mode=") - 1u;
		pos += (size_t)snprintf(buf + pos, space - pos, "%s",
					ctx->parse_mode);
	}

	strcpy(buf + pos, "&text=");
	pos += sizeof("&text=") - 1u;
	urlencode(buf + pos, ctx->text, space - pos, true);

	tg_api_set_method(handle, "sendMessage");
	tg_api_set_body(handle, buf);
	tg_api_post(handle);
}

