

#include <gwbot/common.h>
#include <gwbot/lib/tg_api/send_message.h>


int gwbot_run(struct gwbot_cfg *cfg)
{
	/* TODO: Event handler here... */

	/* Minimal working example */
	tg_api_handle *handle;
	tg_api_smsg msg = {
		.chat_id = -1001422514298,
		.reply_to_msg_id = 0, /* 0 means don't reply to msg */
		.parse_mode = "HTML",
		.text = "AAAAA\n<b>Test HTML parse_mode</b>"
	};


	handle = tg_api_hcreate(cfg->token);
	tga_send_msg(handle, &msg);

	/*
	 * Must destroy the handle if it is not used anymore.
	 */
	tg_api_destroy(handle);

	return 0;
}
