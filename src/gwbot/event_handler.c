
#include <unistd.h>
#include <gwbot/lib/tg_event.h>
#include <gwbot/event_handler.h>
#include <gwbot/lib/tg_api/send_message.h>


static int handle_event_text(struct gwbot_cfg *cfg, struct tgev *evt)
{
	const char *response;
	tg_api_handle *handle;
	struct tga_send_msg smsg_pl;
	struct tgev_text *msg_text = &evt->msg_text;


	smsg_pl = (struct tga_send_msg){
		.chat_id		= msg_text->chat.id,
		.reply_to_msg_id	= msg_text->msg_id,
		.text			= "Hello World!",
		.parse_mode		= PARSE_MODE_HTML
	};

	handle = tg_api_hcreate(cfg->cred.token);

	if (tg_api_send_msg(handle, &smsg_pl) == 0) {
		response = tg_api_get_body(handle);
		printf("%s\n", response);
	} else {
		pr_err("tg_api_send_msg()");
	}

	tg_api_destroy(handle);

	return 0;
}


int gwbot_event_handler(struct gwbot_cfg *cfg, const char *json_str)
{
	int ret;
	struct tgev evt;

	ret = tg_event_load_str(json_str, &evt);
	if (unlikely(ret < 0)) {
		pr_err("tg_event_load_str(): " PRERF, PREAR(-ret));
		return ret;
	}


	switch (evt.type) {
	case TGEV_UNKNOWN:
		prl_notice(5, "Got TGEV_UNKNOWN");
		break;
	case TGEV_TEXT:
		prl_notice(5, "Got TGEV_TEXT");
		ret = handle_event_text(cfg, &evt);
		break;
	case TGEV_PHOTO:
		prl_notice(5, "Got TGEV_PHOTO");
		break;
	case TGEV_STICKER:
		prl_notice(5, "Got TGEV_STICKER");
		break;
	default:
		pr_err("Invalid event");
		break;
	}

	prl_notice(5, "Finished!");
	tg_event_destroy(&evt);
	return ret;
}
