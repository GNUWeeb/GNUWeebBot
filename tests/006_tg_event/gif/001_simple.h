

static TEATEST(006_tg_event_gif, 001_simple)
{
	TQ_START;
	int ret = 0;
	struct tgev evt;
	char *json_str = NULL;
	struct tgevi_entity *entity = NULL;
	struct tgev_gif *msg_gif = NULL;
	struct tgevi_from *from = NULL;
	struct tgevi_from *fwd_from = NULL;
	struct tgevi_chat *chat = NULL;
	struct tgevi_media *anim = NULL;
	struct tgevi_file *doc = NULL;

	TQ_VOID(json_str = load_str_from_file("gif/001_simple.json"));
	TQ_VOID(ret = tg_event_load_str(json_str, &evt));
	TQ_VOID(free(json_str));


	TQ_ASSERT((ret == 0));
	TQ_ASSERT((evt.type == TGEV_GIF));
	TQ_ASSERT((evt.json != NULL));
	TQ_ASSERT((evt.update_id == 346094824));


	TQ_VOID(msg_gif = &evt.msg_gif);
	TQ_ASSERT((msg_gif->msg_id == 12100));


	TQ_VOID(from = &msg_gif->from);
	TQ_ASSERT((from->id == 243692601));
	TQ_ASSERT((from->is_bot == false));
	TQ_ASSERT((!nnstrcmp(from->first_name, "io_uring_enter")));
	TQ_ASSERT(from->last_name == NULL);
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")));
	TQ_ASSERT((!nnstrcmp(from->lang, "en")));


	TQ_VOID(chat = &msg_gif->chat);
	TQ_ASSERT((chat->id == -1001226735471));
	TQ_ASSERT((!nnstrcmp(chat->title, "Private Cloud")));
	TQ_ASSERT(chat->username == NULL);
	TQ_ASSERT((chat->type == TGEV_CHAT_SUPERGROUP));


	TQ_ASSERT((msg_gif->date == 1617101917));



	TQ_VOID(doc = &msg_gif->document);
	TQ_ASSERT(doc != NULL);
	TQ_ASSERT(!nnstrcmp(doc->file_name, "mp4.mp4"));
	TQ_ASSERT(!nnstrcmp(doc->mime_type, "video/mp4"));
	TQ_ASSERT(!nnstrcmp(doc->file_id, "CgACAgQAAx0CSR5_bwACL0RgYwRdxyKbZSwuh51xDOb89FpcdgACPgIAAuVylVLLoiSQDGZCNh4E"));
	TQ_ASSERT(!nnstrcmp(doc->file_unique_id, "AgADPgIAAuVylVI"));
	TQ_ASSERT(doc->file_size == 76923);
	TQ_ASSERT(doc->width == 0);
	TQ_ASSERT(doc->height == 0);



	TQ_VOID(anim = &msg_gif->animation);
	TQ_ASSERT(anim != NULL);
	TQ_ASSERT(anim->title == NULL);
	TQ_ASSERT(!nnstrcmp(anim->file_name, "mp4.mp4"));
	TQ_ASSERT(!nnstrcmp(anim->mime_type, "video/mp4"));
	TQ_ASSERT(anim->duration == 5);
	TQ_ASSERT(!nnstrcmp(anim->file_id, "CgACAgQAAx0CSR5_bwACL0RgYwRdxyKbZSwuh51xDOb89FpcdgACPgIAAuVylVLLoiSQDGZCNh4E"));
	TQ_ASSERT(!nnstrcmp(anim->file_unique_id, "AgADPgIAAuVylVI"));
	TQ_ASSERT(anim->file_size == 76923);
	TQ_ASSERT(anim->width == 320);
	TQ_ASSERT(anim->height == 240);





	TQ_ASSERT((!nnstrcmp(msg_gif->caption,
		"test hahaha huhuhu hmmmm")));
	TQ_ASSERT((msg_gif->caption_entity_c == 4));


	TQ_VOID(entity = msg_gif->caption_entities);
	TQ_ASSERT(
		   (entity != NULL)
		&& (entity[0].offset == 0)
		&& (entity[0].length == 4)
		&& (!nnstrcmp(entity[0].type, "bold"))


		&& (entity[1].offset == 5)
		&& (entity[1].length == 6)
		&& (!nnstrcmp(entity[1].type, "code"))


		&& (entity[2].offset == 12)
		&& (entity[2].length == 6)
		&& (!nnstrcmp(entity[2].type, "italic"))

		&& (entity[3].offset == 19)
		&& (entity[3].length == 5)
		&& (!nnstrcmp(entity[3].type, "strikethrough"))
	);

	/* 
	 * TODO:
	 *	create reply_to unit test.
	 */
	TQ_ASSERT(msg_gif->reply_to == NULL);

	TQ_VOID(tg_event_destroy(&evt));
	TQ_RETURN;
}
