
#include <stdio.h>
#include <gwbot/common.h>
#include <gwbot/lib/arena.h>
#include <gwbot/lib/tg_api/send_message.h>
#include <json-c/json.h>

static void myJsonTest();

int main(int argc, char *argv[])
{
        myJsonTest();
        return 0;


	int ret;
	struct gwbot_cfg cfg;
	char arena_buffer[4096];

	memset(&cfg, 0, sizeof(cfg));
	memset(arena_buffer, 0, sizeof(arena_buffer));
	ar_init(arena_buffer, sizeof(arena_buffer));

	/* Parse command line arguments */
	ret = gwbot_argv_parse(argc, argv, &cfg);
	if (ret != 0)
		return ret;

	if (cfg.cfg_file != NULL) {
		/* Parse config file */
		ret = gwbot_cfg_parse(&cfg);
		if (ret != 0)
			return ret;
	}

	ret = gwbot_run(&cfg);
	if (unlikely(ret != 0)) {
		printf("\n%s --help for usage information\n\n", argv[0]);
	}

	return ret;
}

static void myJsonTest()
{
        FILE *fpo;
        //char buffer[1024] = "{\"update_id\":346088118,\"message\":\"HELLO!\"}";
        char buffer[2048];
        fpo = fopen("/home/mrwoto/Ali/Programming/json/bullshits/telegrams/recieving/texts/text1.json", "r");
        fread(buffer, 1024, 1, fpo);
        fclose(fpo);
	struct json_object *parsed_json;
	struct json_object *update_id;
	struct json_object *message;
	//struct json_object *friends;
	struct json_object *message_id;
	//size_t n_friends;

	//size_t i;	


	parsed_json = json_tokener_parse(buffer);
	json_object_object_get_ex(parsed_json, "update_id", &update_id);
	json_object_object_get_ex(parsed_json, "message", &message);
        json_object_object_get_ex(message, "message_id", &message_id);
	//json_object_object_get_ex(parsed_json, "friends", &friends);
        //printf(buffer);
	printf("update_id: %s\n", json_object_get_string(update_id));
	printf("message_id: %s\n", json_object_get_string(message_id));

	//n_friends = json_object_array_length(friends);
	//printf("Found %lu friends\n",n_friends);

	//for(i=0;i<n_friends;i++) {
	//	//friend = json_object_array_get_idx(friends, i);
	//	printf("%lu. %s\n",i+1,json_object_get_string(friend));
	//}	
}