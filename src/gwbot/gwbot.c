

#include <gwbot/common.h>
#include <gwbot/lib/tg_api/send_message.h>


int gwbot_run(struct gwbot_cfg *cfg)
{
	/* TODO: Create an event handler here... */




	/* Minimal working example */
	tg_api_handle *handle;

	tg_api_smsg msg = {
		.chat_id = -1001422514298, /* GNU/Weeb TDD group */
		.reply_to_msg_id = 312, /* 0 means don't reply to msg */
		.parse_mode = PARSE_MODE_HTML,
		.text = "AAAAA\n<b>Test reply to message</b>"
	};


	handle = tg_api_hcreate(cfg->token);
	tga_send_msg(handle, &msg);

	printf("%s\n", tg_api_res_get_body(&handle->res));


	/*
	 * Must destroy the handle if it is not used anymore.
	 */
	tg_api_destroy(handle);

	return 0;
}
