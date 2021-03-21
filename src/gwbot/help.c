
#include <gwbot/common.h>


void print_gwbot_help(const char *app)
{
	printf("GNUWeebBot " GWBOT_VERSION "\n\n");
	printf("Usage: %s -c <config_file>\n", app);
	printf("\n");
	printf("[Basic]\n");
	printf("  -h, --help\t\t\tShow this help message\n");
	printf("  -v, --version\t\t\tShow app version\n");
	printf("  -c, --config=FILE\t\tSet config file\n");
	printf("\n");
	printf("[Credential]\n");
	printf("  -t, --token=TOKEN\t\tSet bot token\n");
	printf("\n");
}
