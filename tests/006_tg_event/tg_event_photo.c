
#include <libgen.h>
#include <assert.h>
#include "006_tg_event.h"
#include <gwbot/lib/tg_event.h>




static int test_tg_event_photo_001_simple(void)
{
	int ret;
	char *json_str;
	struct tgev evt;
	struct tgevi_entity *entity;
	struct tgev_text *msg_text;
	struct tgevi_from *from;
	struct tgevi_chat *chat;

	json_str = load_str_from_file("tg_event_photo/001_simple.json");
	ret = tg_event_load_str(json_str, &evt);
	free(json_str);

	tg_event_destroy(&evt);

	return 0;
}


struct list_func tg_event_text_list[] = {
	{test_tg_event_photo_001_simple, 220},
	{NULL, 0}
};

int tg_event_text(void)
{
	size_t num = sizeof(tg_event_text_list) / sizeof(*tg_event_text_list);
	for (size_t i = 0; i < num; i++) {
		if (tg_event_text_list[i].func)
			tg_event_text_list[i].func();
	}

	return 0;
}
