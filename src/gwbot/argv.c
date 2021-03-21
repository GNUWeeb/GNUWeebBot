
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <gwbot/common.h>
#include <gwbot/lib/arena.h>
#include <gwbot/lib/string.h>


struct parse_struct {
	char			*app;
	struct gwbot_cfg	*cfg;
};


static const struct option long_opt[] = {
	{"help",          no_argument,       0, 'h'},
	{"version",       no_argument,       0, 'v'},
	{"thread",        required_argument, 0, 't'},
	{"config",        required_argument, 0, 'c'},
	{"data-dir",      required_argument, 0, 'D'},
	{0, 0, 0, 0}
};


static const char short_opt[] = "hvt:c:D:";


static __always_inline int getopt_handler(int argc, char *argv[],
					  struct parse_struct *ctx)
{
	int c;
	char *app = ctx->app;
	struct gwbot_cfg *cfg = ctx->cfg;

	while (true) {
		int option_index = 0;

		c = getopt_long(argc, argv, short_opt, long_opt, &option_index);
		if (unlikely(c == -1))
			break;

		switch (c) {
		case 'h':
			print_gwbot_help(app);
			goto out_exit;
		case 'v':
			break;
		case 't':
			break;
		case 'c':
			break;
		case 'D':
			break;
		default:
			return -EINVAL;
		}
	}

	return 0;
out_exit:
	exit(0);
}


int gwbot_argv_parse(int argc, char *argv[], struct gwbot_cfg *cfg)
{
	int ret;
	struct parse_struct ctx;

	ctx.app = argv[0];
	ctx.cfg = cfg;
	ret = getopt_handler(argc - 1, argv + 1, &ctx);
	if (ret < 0)
		return ret;

	return 0;
}
