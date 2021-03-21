
#include <stdio.h>
#include <gwbot/common.h>


int main(int argc, char *argv[])
{
	int ret;
	struct gwbot_cfg cfg;

	if (argc == 1) {
		print_gwbot_help(argv[0]);
		return 0;
	}

	ret = gwbot_argv_parse(argc, argv, &cfg);
	if (ret != 0)
		return ret;


	return 0;
}
