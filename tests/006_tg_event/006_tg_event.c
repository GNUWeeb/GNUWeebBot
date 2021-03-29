

#include <teatest.h>
#include <gwbot/lib/tg_event.h>


char *load_str_from_file(const char *file);
int nnstrcmp(const char *s1, const char *s2);


#include "photo/001_simple.h"
#include "sticker/001_simple.h"
#include "text/001_simple.h"
#include "text/003_private.h"


static const test_entry_t entry[] = {
	FN_TEATEST(006_tg_event_photo, 001_simple),

	FN_TEATEST(006_tg_event_sticker, 001_simple),

	FN_TEATEST(006_tg_event_text, 001_simple),
	FN_TEATEST(006_tg_event_text, 003_private),
	NULL
};

int main(int argc, char *argv[])
{
	int ret;

	if (argc > 1)
		return spawn_valgrind(argc, argv);

	ret = init_test(entry);
	if (ret != 0)
		return ret;

	ret = run_test(entry);
	return ret;
}


int nnstrcmp(const char *s1, const char *s2)
{
	if (s1 == NULL || s1 == NULL)
		return -1;

	return strcmp(s1, s2);
}

char *load_str_from_file(const char *file)
{
	int err;
	long pos;
	FILE *handle;
	char *ret = NULL;
	char filename[1024];

	snprintf(filename, sizeof(filename), THIS_TEST_BASEPATH "/%s", file);

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
