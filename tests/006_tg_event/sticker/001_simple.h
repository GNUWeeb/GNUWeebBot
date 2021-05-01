

static TEATEST(006_tg_event_sticker, 001_simple)
{
	TQ_START;
	int ret = 0;
	struct tgev evt, *reply_to = NULL;
	char *json_str = NULL;
	struct tgev_sticker *msg_sticker = NULL;
	struct tgevi_from *from = NULL;
	struct tgevi_chat *chat = NULL;
	struct tgevi_sticker *sticker = NULL;
	struct tgevi_file *thumb = NULL;
	struct tgev_text *msg_text = NULL;
	struct tgevi_entity *entity = NULL;

	TQ_VOID(json_str = load_str_from_file("sticker/001_simple.json"));
	TQ_VOID(ret = tg_event_load_str(json_str, &evt));
	TQ_VOID(free(json_str));


	TQ_ASSERT((ret == 0));
	TQ_ASSERT((evt.type == TGEV_STICKER));
	TQ_ASSERT((evt.json != NULL));
	TQ_ASSERT((evt.update_id == 346091791));


	msg_sticker = &evt.msg_sticker;
	TQ_ASSERT((msg_sticker->msg_id == 12011));


	from = &msg_sticker->from;
	TQ_ASSERT((from->id == 243692601));
	TQ_ASSERT((from->is_bot == false));
	TQ_ASSERT((!nnstrcmp(from->first_name, "io_uring_enter")));
	TQ_ASSERT(from->last_name == NULL);
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")));
	TQ_ASSERT((!nnstrcmp(from->lang, "en")));


	chat = &msg_sticker->chat;
	TQ_ASSERT((chat->id == -1001226735471));
	TQ_ASSERT((!nnstrcmp(chat->title, "Private Cloud")));
	TQ_ASSERT(chat->username == NULL);
	TQ_ASSERT((chat->type == TGEV_CHAT_SUPERGROUP));


	TQ_ASSERT((msg_sticker->date == 1616771587));

	sticker = &msg_sticker->sticker;
	TQ_ASSERT(sticker != NULL);
	TQ_ASSERT(!nnstrcmp(sticker->file_id, "CAACAgUAAx0CSR5_bwACLutgXfoDWveHyUMoZXlybtyIF5Ge0gACsAEAAr15CFRC9ndUxCCWJh4E"));
	TQ_ASSERT(!nnstrcmp(sticker->file_unique_id, "AgADsAEAAr15CFQ"));
	TQ_ASSERT(sticker->file_size == 4964);
	TQ_ASSERT(sticker->width == 460);
	TQ_ASSERT(sticker->height == 512);
	TQ_ASSERT(!nnstrcmp(sticker->emoji, "\xf0\x9f\x98\xad"));
	TQ_ASSERT(!nnstrcmp(sticker->set_name, "Mysticial001"));
	TQ_ASSERT(sticker->is_animated == false);
	

	thumb = &sticker->thumb;
	TQ_ASSERT(thumb != NULL);
	TQ_ASSERT(!nnstrcmp(thumb->file_id, "AAMCBQADHQJJHn9vAAIu62Bd-gNa94fJQyhleXJu3IgXkZ7SAAKwAQACvXkIVEL2d1TEIJYmXjY8cHQAAwEAB20AA2d_AAIeBA"));
	TQ_ASSERT(!nnstrcmp(thumb->file_unique_id, "AQADXjY8cHQAA2d_AAI"));
	TQ_ASSERT(thumb->file_size == 3314);
	TQ_ASSERT(thumb->width == 287);
	TQ_ASSERT(thumb->height == 320);

	TQ_ASSERT((reply_to = msg_sticker->reply_to));

	/*
	 * Reply to
	 */
	TQ_ASSERT((reply_to->type == TGEV_TEXT));
	TQ_VOID(msg_text = &reply_to->msg_text);

	TQ_ASSERT((msg_text->msg_id == 11943));

	TQ_VOID(from = &msg_text->from);
	TQ_ASSERT((from->id == 243692601));
	TQ_ASSERT((from->is_bot == false));
	TQ_ASSERT((!nnstrcmp(from->first_name, "io_uring_enter")));
	TQ_ASSERT((from->last_name == NULL));
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")));
	TQ_ASSERT((!nnstrcmp(from->lang, "en")));

	TQ_VOID(chat = &msg_text->chat);
	TQ_ASSERT((chat->id == -1001226735471l));
	TQ_ASSERT((!nnstrcmp(chat->title, "Private Cloud")));
	TQ_ASSERT((chat->username == NULL));
	TQ_ASSERT((chat->type == TGEV_CHAT_SUPERGROUP));


	TQ_ASSERT((msg_text->date == 1616769780));
	TQ_ASSERT((!nnstrcmp(msg_text->text, "/debug replied msg sticker")));
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
