
#include <gwbot/base.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/restrict_chat_member.h>


int tga_restrict_chat_member(tga_handle_t *handle, const tga_restrict_cm_t *ctx)
{
	char buf[512];
	size_t pos = 0;
	const size_t space = sizeof(buf);
	const tga_res_perm_t *perm = &ctx->permissions;

	pos += (size_t)snprintf(buf + pos, space,
		 /*
		  * Building JSON object is expensive.
		  *
		  * This JSON is so simple that we can
		  * easily use snprintf format as template.
		  */
		 "{"
		 	"\"user_id\":%" PRIu64 ","
		 	"\"chat_id\":%" PRId64 ","
		 	"\"until_date\":%" PRIu64 ","
			"\"permissions\":{"
				"\"can_send_messages\":%s,"
				"\"can_send_media_messages\":%s,"
				"\"can_send_polls\":%s,"
				"\"can_send_other_messages\":%s,"
				"\"can_add_web_page_previews\":%s,"
				"\"can_change_info\":%s,"
				"\"can_invite_users\":%s,"
				"\"can_pin_messages\":%s"
			"}"
		 "}",
		 ctx->user_id,
		 ctx->chat_id,
		 (uint64_t)ctx->until_date,
		 perm->can_send_messages ? "true" : "false",
		 perm->can_send_media_messages ? "true" : "false",
		 perm->can_send_polls ? "true" : "false",
		 perm->can_send_other_messages ? "true" : "false",
		 perm->can_add_web_page_previews ? "true" : "false",
		 perm->can_change_info ? "true" : "false",
		 perm->can_invite_users ? "true" : "false",
		 perm->can_pin_messages ? "true" : "false");


	handle->content_type = APP_JSON;
	tga_set_body(handle, buf);
	tga_set_method(handle, "restrictChatMember");
	return tg_api_post(handle);
}
