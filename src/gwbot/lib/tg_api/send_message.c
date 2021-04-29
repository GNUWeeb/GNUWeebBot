

#include <gwbot/base.h>
#include <gwbot/lib/string.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/send_message.h>


int tga_send_msg(tga_handle_t *handle, const tga_send_msg_t *ctx)
{
	size_t pos = 0;
	size_t text_len;
	char buf[0x6500];
	const size_t space = sizeof(buf);

	pos = (size_t)snprintf(buf, space, "chat_id=%" PRId64, ctx->chat_id);
	if (ctx->reply_to_msg_id) {
		pos += (size_t)snprintf(buf + pos, space - pos,
					"&reply_to_message_id=%" PRIu64,
					ctx->reply_to_msg_id);
	}

	switch (ctx->parse_mode) {
	case PARSE_MODE_OFF:
		break;
	case PARSE_MODE_HTML:
		text_len = sizeof("&parse_mode=html") - 1u;
		memcpy(buf + pos, "&parse_mode=html", text_len);
		pos += text_len;
		break;
	case PARSE_MODE_MARKDOWN:
		text_len = sizeof("&parse_mode=markdown") - 1u;
		memcpy(buf + pos, "&parse_mode=markdown", text_len);
		pos += text_len;
		break;
	default:
		pr_err("Invalid parse_mode in tga_send_msg (%u)",
		       ctx->parse_mode);
		return -EINVAL;
	}

	strcpy(buf + pos, "&text=");
	pos += sizeof("&text=") - 1u;

	text_len = strnlen(ctx->text, ((space - pos) / 3) - 1);

	/*
	 * TODO: Make urlencode less dangerous
	 */
	urlencode(buf + pos, ctx->text, text_len, true);

	tga_set_body(handle, buf);
	tga_set_method(handle, "sendMessage");
	return tg_api_post(handle);
}
