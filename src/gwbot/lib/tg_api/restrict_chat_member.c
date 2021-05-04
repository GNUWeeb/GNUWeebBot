
#include <gwbot/base.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/restrict_chat_member.h>


int tga_restrict_chat_member(tga_handle_t *handle, const tga_restrict_cm_t *ctx)
{
	char buf[512];
	size_t pos = 0;
	const size_t space = sizeof(buf);
	const tga_res_perm_t *perm = &ctx->permissions;

	pos += (size_t)snprintf(buf, space,
				"chat_id=%" PRId64 "&user_id=%" PRIu64,
				ctx->chat_id,
				ctx->user_id);

	if (ctx->until_date)
		pos += (size_t)snprintf(buf + pos, space - pos,
					"&until_date=%" PRIu64,
					(uint64_t)ctx->until_date);

	memcpy(buf + pos, "permissions=%7B%22can_send_messages%22%3A", 41);
	pos += 41;
	if (perm->can_send_messages) {
		memcpy(buf + pos, "true,", 5);
		pos += 5;
	} else {
		memcpy(buf + pos, "false,", 6);
		pos += 6;
	}


	memcpy(buf + pos, "%7B%22can_send_media_messages%22%3A", 35);
	pos += 35;
	if (perm->can_send_media_messages) {
		memcpy(buf + pos, "true,", 5);
		pos += 5;
	} else {
		memcpy(buf + pos, "false,", 6);
		pos += 6;
	}


	memcpy(buf + pos, "%7B%22can_send_polls%22%3A", 26);
	pos += 26;
	if (perm->can_send_polls) {
		memcpy(buf + pos, "true,", 5);
		pos += 5;
	} else {
		memcpy(buf + pos, "false,", 6);
		pos += 6;
	}


	memcpy(buf + pos, "%7B%22can_send_other_messages%22%3A", 35);
	pos += 35;
	if (perm->can_send_other_messages) {
		memcpy(buf + pos, "true,", 5);
		pos += 5;
	} else {
		memcpy(buf + pos, "false,", 6);
		pos += 6;
	}


	memcpy(buf + pos, "%7B%22can_add_web_page_previews%22%3A", 37);
	pos += 37;
	if (perm->can_add_web_page_previews) {
		memcpy(buf + pos, "true,", 5);
		pos += 5;
	} else {
		memcpy(buf + pos, "false,", 6);
		pos += 6;
	}


	memcpy(buf + pos, "%7B%22can_change_info%22%3A", 27);
	pos += 27;
	if (perm->can_change_info) {
		memcpy(buf + pos, "true,", 5);
		pos += 5;
	} else {
		memcpy(buf + pos, "false,", 6);
		pos += 6;
	}


	memcpy(buf + pos, "%7B%22can_invite_users%22%3A", 28);
	pos += 28;
	if (perm->can_invite_users) {
		memcpy(buf + pos, "true,", 5);
		pos += 5;
	} else {
		memcpy(buf + pos, "false,", 6);
		pos += 6;
	}


	memcpy(buf + pos, "%7B%22can_pin_messages%22%3A", 28);
	pos += 28;
	if (perm->can_pin_messages) {
		memcpy(buf + pos, "true", 4);
		pos += 4;
	} else {
		memcpy(buf + pos, "false", 5);
		pos += 5;
	}

	memcpy(buf + pos, "%7D", 4);


	tga_set_body(handle, buf);
	tga_set_method(handle, "restrictChatMember");
	return tg_api_post(handle);	
}