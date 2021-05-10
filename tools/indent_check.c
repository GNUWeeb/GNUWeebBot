
#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int indent_check(const char *fn)
{
	int ret = 0;
	uint32_t lineno = 0;
	char buffer[8192], *bp;
	FILE *handle = fopen(fn, "rb");

	// printf("Checking %s\n", fn);

	if (!handle) {
		ret = errno;
		printf("Error: fopen(\"%s\", \"rb\"): %s\n", fn, strerror(ret));
		return -ret;
	}

	while ((bp = fgets(buffer, sizeof(buffer), handle))) {
		lineno++;
		if (bp[0] == ' ' && bp[1] != '*') {
			ret = 1;
			printf("  Bad indentation char detected: %s:%u\n", fn,
			       lineno);
		}
	}
	fclose(handle);

	return ret;
}


static int scan_rec(const char *dn)
{
	bool stop = false;
	int ret = 0, tmp, n;
	struct dirent **namelist;

	n = scandir(dn, &namelist, NULL, alphasort);
	if (n == -1) {
		ret = errno;
		printf("Error: scandir(\"%s\"): %s\n", dn, strerror(ret));
		return -ret;
	}

	while (n--) {
		
		char fn_buf[4096];
		const char *fn = namelist[n]->d_name;
		size_t len = strlen(fn);
		struct stat path_stat;

		if (len < 3)
			goto next_while;

		snprintf(fn_buf, sizeof(fn_buf), "%s/%s", dn, fn);

		stat(fn_buf, &path_stat);
		if (S_ISDIR(path_stat.st_mode)) {
			tmp = scan_rec(fn_buf);
			if (tmp) {
				if (!stop)
					ret = tmp;
				stop = true;
			}
			goto next_while;
		}

		char a = fn[len - 1], b = fn[len - 2];
		if (!((a == 'c' || a == 'h') && (b == '.')))
			goto next_while;

		tmp = indent_check(fn_buf);
		if (tmp) {
			if (!stop)
				ret = tmp;
			stop = true;
		}

	next_while:
		free(namelist[n]);
	}
	free(namelist);

	return ret;
}


int main(int argc, const char **argv)
{
	if (argc != 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	return scan_rec(argv[1]);
}
