

static TEATEST(006_tg_event_text, 003_private)
{
	TQ_START;
	int ret = 0;
	struct tgev evt, *reply_to;
	char *json_str = NULL;
	struct tgevi_entity *entity = NULL;
	struct tgev_text *msg_text = NULL;
	struct tgevi_from *from = NULL;
	struct tgevi_chat *chat = NULL;


	TQ_VOID(json_str = load_str_from_file("text/003_private.json"));
	TQ_VOID(ret = tg_event_load_str(json_str, &evt));
	TQ_VOID(free(json_str));


	TQ_ASSERT((ret == 0));
	TQ_ASSERT((evt.type == TGEV_TEXT));
	TQ_ASSERT((evt.json != NULL));
	TQ_ASSERT((evt.update_id == 346091871));


	TQ_VOID(msg_text = &evt.msg_text);
	TQ_ASSERT((msg_text->msg_id == 82945));


	TQ_VOID(from = &msg_text->from);
	TQ_ASSERT((from->id == 243692601));
	TQ_ASSERT((from->is_bot == false));
	TQ_ASSERT((!nnstrcmp(from->first_name, "io_uring_enter")));
	TQ_ASSERT((from->last_name == NULL));
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")));
	TQ_ASSERT((!nnstrcmp(from->lang, "en")));


	TQ_VOID(chat = &msg_text->chat);
	TQ_ASSERT((chat->id == 243692601));
	TQ_ASSERT((chat->title == NULL));
	TQ_ASSERT((!nnstrcmp(chat->first_name, "io_uring_enter")));
	TQ_ASSERT((chat->last_name == NULL));
	TQ_ASSERT((!nnstrcmp(chat->username, "ammarfaizi2")));
	TQ_ASSERT((chat->type == TGEV_CHAT_PRIVATE));


	TQ_ASSERT((msg_text->date == 1616779179));
	TQ_ASSERT((!nnstrcmp(msg_text->text, "/debug")));
	TQ_ASSERT((msg_text->entity_c == 1));
	TQ_ASSERT((msg_text->entities != NULL));


	TQ_VOID(entity = &msg_text->entities[0]);
	TQ_ASSERT((entity->offset == 0));
	TQ_ASSERT((entity->length == 6));
	TQ_ASSERT((!nnstrcmp(entity->type, "bot_command")));
	TQ_ASSERT((reply_to = msg_text->reply_to));

	/*
	 * Reply to
	 */
	TQ_ASSERT((reply_to->type == TGEV_TEXT));
	TQ_VOID(msg_text = &reply_to->msg_text);

	TQ_ASSERT((msg_text->msg_id == 82943u));

	TQ_VOID(from = &msg_text->from);
	TQ_ASSERT((from->id == 243692601));
	TQ_ASSERT((from->is_bot == false));
	TQ_ASSERT((!nnstrcmp(from->first_name, "io_uring_enter")));
	TQ_ASSERT((from->last_name == NULL));
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")));
	TQ_ASSERT((!nnstrcmp(from->lang, "en")));

	TQ_VOID(chat = &msg_text->chat);
	TQ_ASSERT((chat->id == 243692601));
	TQ_ASSERT((chat->title == NULL));
	TQ_ASSERT((!nnstrcmp(chat->first_name, "io_uring_enter")));
	TQ_ASSERT((chat->last_name == NULL));
	TQ_ASSERT((!nnstrcmp(chat->username, "ammarfaizi2")));
	TQ_ASSERT((chat->type == TGEV_CHAT_PRIVATE));

	TQ_ASSERT((msg_text->date == 1616779165));
	TQ_ASSERT((!nnstrcmp(msg_text->text, "/debug replied string message")));
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
