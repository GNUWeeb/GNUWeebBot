
#include <stdio.h>
#include <gwbot/common.h>
#include <gwbot/lib/arena.h>
#include <gwbot/lib/tg_event.h>
#include <gwbot/lib/tg_api/send_message.h>


#define TARGET_FILE "tests/006_tg_event_text/data.json"


static char *load_json_str_from_file(const char *str)
{
	int err;
	long pos;
	FILE *handle;
	char *ret = NULL;

	handle = fopen(str, "r");
	if (unlikely(handle == NULL)) {
		err = errno;
		pr_err("fopen(): " PRERF, PREAR(err));
		return NULL;
	}

	if (unlikely(fseek(handle, 0, SEEK_END) != 0)) {
		err = errno;
		pr_err("fseek(): " PRERF, PREAR(err));
		goto out_err_close;
	}

	pos = ftell(handle);
	if (unlikely(pos < 0)) {
		err = errno;
		pr_err("ftell(): " PRERF, PREAR(err));
		goto out_err_close;
	}

	ret = malloc((size_t)pos + 1);
	if (unlikely(ret == NULL)) {
		err = errno;
		pr_err("malloc(): " PRERF, PREAR(err));
		goto out_err_close;
	}

	rewind(handle);
	fread(ret, sizeof(char), (size_t)pos, handle);
	ret[pos] = '\0';
	fclose(handle);
	return ret;

out_err_close:
	free(ret);
	fclose(handle);
	return NULL;
}


int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	char *json_str;
	struct tgev evt;

	json_str = load_json_str_from_file(TARGET_FILE);
	assert(json_str != NULL);

	printf("load() = %d\n", tg_event_load(json_str, &evt));

	printf("msg.text = %s\n", evt.msg_text.text);
	printf("msg.from.id = %lu\n", evt.msg_text.from.id);

	free(json_str);
	tg_event_destroy(&evt);



	// int ret;
	// struct gwbot_cfg cfg;
	// char arena_buffer[4096];

	// memset(&cfg, 0, sizeof(cfg));
	// memset(arena_buffer, 0, sizeof(arena_buffer));
	// ar_init(arena_buffer, sizeof(arena_buffer));

	// /* Parse command line arguments */
	// ret = gwbot_argv_parse(argc, argv, &cfg);
	// if (ret != 0)
	// 	return ret;

	// if (cfg.cfg_file != NULL) {
	// 	/* Parse config file */
	// 	ret = gwbot_cfg_parse(&cfg);
	// 	if (ret != 0)
	// 		return ret;
	// }

	// ret = gwbot_run(&cfg);
	// if (unlikely(ret != 0)) {
	// 	printf("\n%s --help for usage information\n\n", argv[0]);
	// }

	// return ret;
}