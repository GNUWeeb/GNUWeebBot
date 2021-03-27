
#include <assert.h>
#include "006_tg_event.h"
#include <gwbot/lib/tg_event.h>


static int test_tg_event_text_001_simple(void)
{
	int ret;
	char *json_str;
	struct tgev evt;
	struct tgevi_entity *entity;
	struct tgev_text *msg_text;
	struct tgevi_from *from;
	struct tgevi_chat *chat;


	json_str = load_str_from_file("tg_event_text/001_simple.json");
	ret = tg_event_load_str(json_str, &evt);
	free(json_str);


	TQ_ASSERT((ret == 0), 10);
	TQ_ASSERT((evt.type == TGEV_TEXT), 10);
	TQ_ASSERT((evt.json != NULL), 10);
	TQ_ASSERT((evt.update_id == 346089057u), 10);


	msg_text = &evt.msg_text;
	TQ_ASSERT((msg_text->msg_id == 525u), 10);


	from = &msg_text->from;
	TQ_ASSERT((from->id == 243692601ul), 10);
	TQ_ASSERT((from->is_bot == false), 10);
	TQ_ASSERT((!nnstrcmp(from->first_name, "cpu_relax();")), 10);
	TQ_ASSERT((!nnstrcmp(from->last_name, "//")), 10);
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")), 10);
	TQ_ASSERT((!nnstrcmp(from->lang, "en")), 10);


	chat = &msg_text->chat;
	TQ_ASSERT((chat->id == -1001422514298l), 10);
	TQ_ASSERT((!nnstrcmp(chat->title, "GNU/Weeb Test Driven Development")),
		  10);
	TQ_ASSERT((!nnstrcmp(chat->username, "GNUWeebTDD")), 10);
	TQ_ASSERT((chat->type == TGEV_CHAT_SUPERGROUP), 10);


	TQ_ASSERT((msg_text->date == 1616588789ul), 10);
	TQ_ASSERT((!nnstrcmp(msg_text->text, "/debug Hello World!")), 10);
	TQ_ASSERT((msg_text->entity_c == 1), 10);
	TQ_ASSERT((msg_text->entities != NULL), 10);


	entity = &msg_text->entities[0];
	TQ_ASSERT((entity->offset == 0), 10);
	TQ_ASSERT((entity->length == 6), 10);
	TQ_ASSERT((!nnstrcmp(entity->type, "bot_command")), 10);
	TQ_ASSERT((msg_text->reply_to == NULL), 10);


	tg_event_destroy(&evt);
	return 0;
}


static int test_tg_event_text_002_simple(void)
{
	int ret;
	char *json_str;
	struct tgev evt;
	struct tgevi_entity *entity;
	struct tgev_text *msg_text;
	struct tgevi_from *from;
	struct tgevi_chat *chat;


	json_str = load_str_from_file("tg_event_text/003_in_private.json");
	ret = tg_event_load_str(json_str, &evt);
	free(json_str);


	TQ_ASSERT((ret == 0), 10);
	TQ_ASSERT((evt.type == TGEV_TEXT), 10);
	TQ_ASSERT((evt.json != NULL), 10);
	TQ_ASSERT((evt.update_id == 346091871), 10);


	msg_text = &evt.msg_text;
	TQ_ASSERT((msg_text->msg_id == 82945), 10);


	from = &msg_text->from;
	TQ_ASSERT((from->id == 243692601), 10);
	TQ_ASSERT((from->is_bot == false), 10);
	TQ_ASSERT((!nnstrcmp(from->first_name, "io_uring_enter")), 10);
	TQ_ASSERT((from->last_name == NULL), 10);
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")), 10);
	TQ_ASSERT((!nnstrcmp(from->lang, "en")), 10);


	chat = &msg_text->chat;
	TQ_ASSERT((chat->id == 243692601), 10);
	TQ_ASSERT((chat->title == NULL), 10);
	TQ_ASSERT((!nnstrcmp(chat->first_name, "io_uring_enter")), 10);
	TQ_ASSERT((chat->last_name == NULL), 10);
	TQ_ASSERT((!nnstrcmp(chat->username, "ammarfaizi2")), 10);
	TQ_ASSERT((chat->type == TGEV_CHAT_PRIVATE), 10);


	TQ_ASSERT((msg_text->date == 1616779179ul), 10);
	TQ_ASSERT((!nnstrcmp(msg_text->text, "/debug")), 10);
	TQ_ASSERT((msg_text->entity_c == 1), 10);
	TQ_ASSERT((msg_text->entities != NULL), 10);


	entity = &msg_text->entities[0];
	TQ_ASSERT((entity->offset == 0), 10);
	TQ_ASSERT((entity->length == 6), 10);
	TQ_ASSERT((!nnstrcmp(entity->type, "bot_command")), 10);
	TQ_ASSERT((msg_text->reply_to == NULL), 10);


	tg_event_destroy(&evt);
	return 0;
}


struct list_func tg_event_text_list[] = {
	{test_tg_event_text_001_simple, 230},
	{test_tg_event_text_002_simple, 150},
	{NULL, 0}
};
EXPORT_TEST(tg_event_text_list);
