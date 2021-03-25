
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <gwbot/base.h>
#include <gwbot/lib/tg_event.h>
#include <criterion/criterion.h>


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

	ret = malloc((size_t)pos);
	if (unlikely(ret == NULL)) {
		err = errno;
		pr_err("malloc(): " PRERF, PREAR(err));
		goto out_err_close;
	}

	rewind(handle);
	fread(ret, sizeof(char), (size_t)pos, handle);
	fclose(handle);
	return ret;

out_err_close:
	free(ret);
	fclose(handle);
	return NULL;
}


Test(tg_event_text, parse_test)
{
	char *json_str;
	struct tgev evt;

	json_str = load_json_str_from_file(TARGET_FILE);
	cr_assert_neq(json_str, NULL);

	cr_assert(tg_event_load(json_str, &evt) == 0);


	free(json_str);
}
