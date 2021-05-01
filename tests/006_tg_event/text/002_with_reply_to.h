

static TEATEST(006_tg_event_text, 002_with_reply_to)
{
	TQ_START;
	int ret = 0;
	struct tgev evt, *reply_to = NULL;
	char *json_str = NULL;
	struct tgevi_entity *entity = NULL;
	struct tgev_text *msg_text = NULL;
	struct tgevi_from *from = NULL;
	struct tgevi_chat *chat = NULL;


	TQ_VOID(json_str = load_str_from_file("text/002_with_reply_to.json"));
	TQ_VOID(ret = tg_event_load_str(json_str, &evt));
	TQ_VOID(free(json_str));


	TQ_ASSERT((ret == 0));
	TQ_ASSERT((evt.type == TGEV_TEXT));
	TQ_ASSERT((evt.json != NULL));
	TQ_ASSERT((evt.update_id == 346091070u));


	TQ_VOID(msg_text = &evt.msg_text);
	TQ_ASSERT((msg_text->msg_id == 759u));


	TQ_VOID(from = &msg_text->from);
	TQ_ASSERT((from->id == 243692601ul));
	TQ_ASSERT((from->is_bot == false));
	TQ_ASSERT((!nnstrcmp(from->first_name, "io_uring_enter")));
	TQ_ASSERT((from->last_name == NULL));
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")));
	TQ_ASSERT((!nnstrcmp(from->lang, "en")));


	TQ_VOID(chat = &msg_text->chat);
	TQ_ASSERT((chat->id == -1001422514298l));
	TQ_ASSERT((!nnstrcmp(chat->title, "GNU/Weeb Test Driven Development")));
	TQ_ASSERT((!nnstrcmp(chat->username, "GNUWeebTDD")));
	TQ_ASSERT((chat->type == TGEV_CHAT_SUPERGROUP));


	TQ_ASSERT((msg_text->date == 1616742132ul));
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

	TQ_ASSERT((msg_text->msg_id == 758u));

	TQ_VOID(from = &msg_text->from);
	TQ_ASSERT((from->id == 243692601));
	TQ_ASSERT((from->is_bot == false));
	TQ_ASSERT((!nnstrcmp(from->first_name, "io_uring_enter")));
	TQ_ASSERT((from->last_name == NULL));
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")));
	TQ_ASSERT((!nnstrcmp(from->lang, "en")));

	TQ_VOID(chat = &msg_text->chat);
	TQ_ASSERT((chat->id == -1001422514298l));
	TQ_ASSERT((!nnstrcmp(chat->title, "GNU/Weeb Test Driven Development")));
	TQ_ASSERT((!nnstrcmp(chat->username, "GNUWeebTDD")));
	TQ_ASSERT((chat->type == TGEV_CHAT_SUPERGROUP));

	TQ_ASSERT((msg_text->date == 1616742128));
	TQ_ASSERT((!nnstrcmp(msg_text->text, "test reply to")));
	TQ_ASSERT((msg_text->entity_c == 0));
	TQ_ASSERT((msg_text->entities == NULL));

	TQ_VOID(tg_event_destroy(&evt));
	TQ_RETURN;
}
