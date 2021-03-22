
#include <stdio.h>
#include <gwbot/common.h>
#include <gwbot/lib/arena.h>
#include <gwbot/lib/tg_api/send_message.h>


int main(int argc, char *argv[])
{
	int ret;
	struct gwbot_cfg cfg;
	char arena_buffer[4096];

	memset(&cfg, 0, sizeof(cfg));
	memset(arena_buffer, 0, sizeof(arena_buffer));
	ar_init(arena_buffer, sizeof(arena_buffer));

	if (argc == 1) {
		print_gwbot_help(argv[0]);
		return 0;
	}

	/* Parse command line arguments */
	ret = gwbot_argv_parse(argc, argv, &cfg);
	if (ret != 0)
		return ret;

	if (cfg.cfg_file == NULL) {
		pr_err("cfg_file cannot be empty");
		return -1;
	}

	/* Parse config file */
	ret = gwbot_cfg_parse(&cfg);
	if (ret != 0)
		return ret;

	return gwbot_run(&cfg);
}
