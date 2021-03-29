

static TEATEST(006_tg_event_photo, 001_simple)
{
	TQ_START;
	int ret;
	char *json_str;
	struct tgev evt;
	struct tgevi_entity *entity;
	struct tgev_photo *msg_photo;
	struct tgevi_from *from;
	struct tgevi_chat *chat;
	struct tgevi_file *photo;

	TQ_VOID(json_str = load_str_from_file("photo/001_simple.json"));
	TQ_VOID(ret = tg_event_load_str(json_str, &evt));
	TQ_VOID(free(json_str));


	TQ_ASSERT((ret == 0));
	TQ_ASSERT((evt.type == TGEV_PHOTO));
	TQ_ASSERT((evt.json != NULL));
	TQ_ASSERT((evt.update_id == 346089825));


	msg_photo = &evt.msg_photo;
	TQ_ASSERT((msg_photo->msg_id == 576));


	from = &msg_photo->from;
	TQ_ASSERT((from->id == 243692601));
	TQ_ASSERT((from->is_bot == false));
	TQ_ASSERT((!nnstrcmp(from->first_name, "io_uring_enter")));
	TQ_ASSERT((!nnstrcmp(from->last_name, "blabla")));
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")));
	TQ_ASSERT((!nnstrcmp(from->lang, "en")));


	chat = &msg_photo->chat;
	TQ_ASSERT((chat->id == -1001422514298l));
	TQ_ASSERT((!nnstrcmp(chat->title, "GNU/Weeb Test Driven Development")));
	TQ_ASSERT((!nnstrcmp(chat->username, "GNUWeebTDD")));
	TQ_ASSERT((chat->type == TGEV_CHAT_SUPERGROUP));


	TQ_ASSERT((msg_photo->date == 1616657218));


	photo = msg_photo->photo;
	TQ_ASSERT((msg_photo->photo_c == 3));
	TQ_ASSERT(
		   (photo != NULL)
		&& (!nnstrcmp(photo[0].file_id, "AgACAgUAAx0CVMnYegACAkBgXDtCw5SJGFK--L0nuEH9bTHMRQACbqsxG9e54FbXAiO63ePEdZSncnN0AAMBAAMCAANtAAPtMQACHgQ"))
		&& (!nnstrcmp(photo[0].file_unique_id, "AQADlKdyc3QAA-0xAAI"))
		&& (photo[0].file_size == 22345)
		&& (photo[0].width == 320)
		&& (photo[0].height == 180)

		&& (!nnstrcmp(photo[1].file_id, "AgACAgUAAx0CVMnYegACAkBgXDtCw5SJGFK--L0nuEH9bTHMRQACbqsxG9e54FbXAiO63ePEdZSncnN0AAMBAAMCAAN4AAPvMQACHgQ"))
		&& (!nnstrcmp(photo[1].file_unique_id, "AQADlKdyc3QAA-8xAAI"))
		&& (photo[1].file_size == 86886)
		&& (photo[1].width == 800)
		&& (photo[1].height == 450)

		&& (!nnstrcmp(photo[2].file_id, "AgACAgUAAx0CVMnYegACAkBgXDtCw5SJGFK--L0nuEH9bTHMRQACbqsxG9e54FbXAiO63ePEdZSncnN0AAMBAAMCAAN5AAPuMQACHgQ"))
		&& (!nnstrcmp(photo[2].file_unique_id, "AQADlKdyc3QAA-4xAAI"))
		&& (photo[2].file_size == 152004)
		&& (photo[2].width == 1280)
		&& (photo[2].height == 720)
	);


	TQ_ASSERT((!nnstrcmp(msg_photo->caption,
		"test photo with caption with entities\n/botcmd\nbold\nitalic")));
	TQ_ASSERT((msg_photo->caption_entity_c == 3));


	entity = msg_photo->caption_entities;
	TQ_ASSERT(
		   (entity != NULL)
		&& (entity[0].offset == 38)
		&& (entity[0].length == 7)
		&& (!nnstrcmp(entity[0].type, "bot_command"))


		&& (entity[1].offset == 46)
		&& (entity[1].length == 4)
		&& (!nnstrcmp(entity[1].type, "bold"))


		&& (entity[2].offset == 51)
		&& (entity[2].length == 6)
		&& (!nnstrcmp(entity[2].type, "italic"))
	);


	TQ_ASSERT((msg_photo->reply_to == NULL));

	TQ_VOID(tg_event_destroy(&evt));
	TQ_RETURN;
}
