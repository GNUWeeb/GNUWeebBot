

static TEATEST(006_tg_event_text, 001_simple)
{
	TQ_START;
	int ret = 0;
	struct tgev evt;
	char *json_str = NULL;
	struct tgevi_entity *entity = NULL;
	struct tgev_text *msg_text = NULL;
	struct tgevi_from *from = NULL;
	struct tgevi_chat *chat = NULL;


	TQ_VOID(json_str = load_str_from_file("text/001_simple.json"));
	TQ_VOID(ret = tg_event_load_str(json_str, &evt));
	TQ_VOID(free(json_str));


	TQ_ASSERT((ret == 0));
	TQ_ASSERT((evt.type == TGEV_TEXT));
	TQ_ASSERT((evt.json != NULL));
	TQ_ASSERT((evt.update_id == 346089057u));


	TQ_VOID(msg_text = &evt.msg_text);
	TQ_ASSERT((msg_text->msg_id == 525u));


	TQ_VOID(from = &msg_text->from);
	TQ_ASSERT((from->id == 243692601ul));
	TQ_ASSERT((from->is_bot == false));
	TQ_ASSERT((!nnstrcmp(from->first_name, "cpu_relax();")));
	TQ_ASSERT((!nnstrcmp(from->last_name, "//")));
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")));
	TQ_ASSERT((!nnstrcmp(from->lang, "en")));


	TQ_VOID(chat = &msg_text->chat);
	TQ_ASSERT((chat->id == -1001422514298l));
	TQ_ASSERT((!nnstrcmp(chat->title, "GNU/Weeb Test Driven Development")));
	TQ_ASSERT((!nnstrcmp(chat->username, "GNUWeebTDD")));
	TQ_ASSERT((chat->type == TGEV_CHAT_SUPERGROUP));


	TQ_ASSERT((msg_text->date == 1616588789ul));
	TQ_ASSERT((!nnstrcmp(msg_text->text, "/debug Hello World!")));
	TQ_ASSERT((msg_text->entity_c == 1));
	TQ_ASSERT((msg_text->entities != NULL));


	TQ_VOID(entity = &msg_text->entities[0]);
	TQ_ASSERT((entity->offset == 0));
	TQ_ASSERT((entity->length == 6));
	TQ_ASSERT((!nnstrcmp(entity->type, "bot_command")));
	TQ_ASSERT((msg_text->reply_to == NULL));


	TQ_VOID(tg_event_destroy(&evt));
	TQ_RETURN;
}
