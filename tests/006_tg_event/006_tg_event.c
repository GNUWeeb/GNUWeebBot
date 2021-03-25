
#include <signal.h>
#include "006_tg_event.h"

uint32_t passed = 0;
uint32_t credit = 0;
uint32_t total_credit = 0;

static void sig_handler(int sig);
static void print_info(int ret);

int main()
{
	int ret;
	pr_notice("Running tg_event test...");

	signal(SIGSEGV, sig_handler);
	signal(SIGABRT, sig_handler);

	ret = tg_event_text();
	if (unlikely(ret != 0))
		goto out;

out:
	print_info(ret);
	return ret;
}


char *load_str_from_file(const char *file)
{
	int err;
	long pos;
	FILE *handle;
	char *ret = NULL;
	char filename[1024];

	snprintf(filename, sizeof(filename), MY_BASEPATH "/%s", file);

	handle = fopen(filename, "r");
	if (unlikely(handle == NULL)) {
		err = errno;
		pr_err("fopen(\"%s\"): " PRERF, filename, PREAR(err));
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


static void sig_handler(int sig)
{
	if (sig == SIGSEGV) {
		core_dump();
		panic("SIGSEGV caught!");
		pr_emerg("HEY, WHAT THE F*UCK!");
		pr_emerg("You crashed the program memory!");
		pr_emerg("Segmentation Fault (core dumped)");
		pr_emerg("===============================================");
		exit(1);
	} else if (sig == SIGABRT) {
		core_dump();
		panic("SIGABRT caught!");
		pr_emerg("What the hell did you do?");
		pr_emerg("Aborted (core dumped)");
		pr_emerg("===============================================");
		exit(1);
	}
	exit(0);
}


static void print_info(int ret)
{
	double accuracy;

	if (credit == 0 || total_credit == 0) {
		accuracy = 0;
	} else {
		accuracy = ((double)credit / (double)total_credit) * 100.0;
	}

	printf("==================================================\n");
	printf("Last return value\t: %d\n", ret);
	printf("Your accuracy\t\t: %.3f\n", accuracy);
	printf("Earned credit\t\t: %u\n", credit);
	printf("Total credit\t\t: %u\n", total_credit);
	printf("==================================================\n");
}
