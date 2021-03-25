

#include "006_tg_event.h"
#include <gwbot/lib/tg_event.h>


int tg_event_text(void)
{
	char *json_str;
	json_str = load_str_from_file("tg_event_text/001_simple.json");
	free(json_str);
	return 0;
}
