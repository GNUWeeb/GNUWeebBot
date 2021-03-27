
#include <assert.h>
#include "006_tg_event.h"
#include <gwbot/lib/tg_event.h>


static int test_tg_event_sticker_001_simple(void)
{
	int ret;
	char *json_str;
	struct tgev evt;
	struct tgevi_entity *entity;
	struct tgev_sticker *msg_sticker;
	struct tgevi_from *from;
	struct tgevi_chat *chat;
	struct tgevi_sticker *sticker;
	struct tgevi_file *thumb;

	memset(&evt, 0, sizeof(evt));

	json_str = load_str_from_file("tg_event_sticker/001_simple.json");
	ret = tg_event_load_str(json_str, &evt);
	free(json_str);


	TQ_ASSERT((ret == 0), 10);
	TQ_ASSERT((evt.type == TGEV_STICKER), 10);
	TQ_ASSERT((evt.json != NULL), 10);
	TQ_ASSERT((evt.update_id == 346091791), 10);


	msg_sticker = &evt.msg_sticker;
	TQ_ASSERT((msg_sticker->msg_id == 12011), 10);


	from = &msg_sticker->from;
	TQ_ASSERT((from->id == 243692601), 10);
	TQ_ASSERT((from->is_bot == false), 10);
	TQ_ASSERT((!nnstrcmp(from->first_name, "io_uring_enter")), 10);
	TQ_ASSERT(from->last_name == NULL, 10);
	TQ_ASSERT((!nnstrcmp(from->username, "ammarfaizi2")), 10);
	TQ_ASSERT((!nnstrcmp(from->lang, "en")), 10);


	chat = &msg_sticker->chat;
	TQ_ASSERT((chat->id == -1001226735471), 10);
	TQ_ASSERT((!nnstrcmp(chat->title, "Private Cloud")),
		  10);
	TQ_ASSERT(chat->username == NULL, 10);
	TQ_ASSERT((chat->type == TGEV_CHAT_SUPERGROUP), 10);


	TQ_ASSERT((msg_sticker->date == 1616771587), 10);

	sticker = &msg_sticker->sticker;
	TQ_ASSERT(sticker != NULL, 10);
	TQ_ASSERT(!nnstrcmp(sticker->file_id, "CAACAgUAAx0CSR5_bwACLutgXfoDWveHyUMoZXlybtyIF5Ge0gACsAEAAr15CFRC9ndUxCCWJh4E"), 10);
	TQ_ASSERT(!nnstrcmp(sticker->file_unique_id, "AgADsAEAAr15CFQ"), 10);
	TQ_ASSERT(sticker->file_size == 4964, 10);
	TQ_ASSERT(sticker->width == 460, 10);
	TQ_ASSERT(sticker->height == 512, 10);
	TQ_ASSERT(!nnstrcmp(sticker->emoji, "\xf0\x9f\x98\xad"), 10);
	TQ_ASSERT(!nnstrcmp(sticker->set_name, "Mysticial001"), 10);
	TQ_ASSERT(sticker->is_animated == false, 10);
	

	thumb = &sticker->thumb;
	TQ_ASSERT(thumb != NULL, 10);
	TQ_ASSERT(!nnstrcmp(thumb->file_id, "AAMCBQADHQJJHn9vAAIu62Bd-gNa94fJQyhleXJu3IgXkZ7SAAKwAQACvXkIVEL2d1TEIJYmXjY8cHQAAwEAB20AA2d_AAIeBA"), 10);
	TQ_ASSERT(!nnstrcmp(thumb->file_unique_id, "AQADXjY8cHQAA2d_AAI"), 10);
	TQ_ASSERT(thumb->file_size == 3314, 10);
	TQ_ASSERT(thumb->width == 287, 10);
	TQ_ASSERT(thumb->height == 320, 10);


	TQ_ASSERT((msg_sticker->reply_to == NULL), 10);
	tg_event_destroy(&evt);
	return 0;
}


struct list_func tg_event_sticker_list[] = {
	{test_tg_event_sticker_001_simple, 320},
	{NULL, 0}
};
EXPORT_TEST(tg_event_sticker_list);
