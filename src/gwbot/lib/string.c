// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/lib/string.c
 *
 *  String helpers for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <gwbot/base.h>
#include <gwbot/lib/string.h>

#ifdef __SSE2__
#  include <emmintrin.h>
#endif


/**
 * Thanks to PHP
 * https://github.com/php/php-src/blob/e9d78339e7ff2edb8a1eda93d047ccaac25efa24/ext/standard/exec.c#L388-L468
 */
char *escapeshellarg(char *alloc, const char *str, size_t len, size_t *res_len)
{
	size_t y = 0;
	size_t l = (len > 0) ? len : strlen(str);
	size_t x;
	char   *cmd;

	if (alloc == NULL) {
		/* Worst case */
		cmd = (char *)malloc((sizeof(char) * l * 4) + 1);
	} else {
		cmd = alloc;
	}

#ifdef WIN32
	cmd[y++] = '"';
#else
	cmd[y++] = '\'';
#endif

	for (x = 0; x < l; x++) {
		switch (str[x]) {
#ifdef WIN32
		case '"':
		case '%':
		case '!':
			cmd[y++] = ' ';
			break;
#else
		case '\'':
			cmd[y++] = '\'';
			cmd[y++] = '\\';
			cmd[y++] = '\'';
#endif
		fallthrough;
		default:
			cmd[y++] = str[x];
		}
	}

#ifdef WIN32
	if (y > 0 && '\\' == cmd[y - 1]) {
		int k = 0, n = y - 1;
		for (; n >= 0 && '\\' == cmd[n]; n--, k++);
		if (k % 2) {
			cmd[y++] = '\\';
		}
	}
	cmd[y++] = '"';
#else
	cmd[y++] = '\'';
#endif

	cmd[y] = '\0';

	if (res_len != NULL)
		*res_len = y;

	return cmd;
}


#define HEQ(C) ((*head) == (C))
#define TEQ(C) ((*tail) == (C))


char *trim_len(char *head, size_t len, size_t *res_len)
{
	char *tail  = &(head[len - 1]);
	bool move_t = false;

	while ((len > 0) && (HEQ(' ') || HEQ('\n') || HEQ('\r') || HEQ('\v'))) {
		head++;
		len--;
	}

	while ((len > 0) && (TEQ(' ') || TEQ('\n') || TEQ('\r') || TEQ('\v'))) {
		tail--;
		len--;
		move_t = true;
	}

	if ((len > 0) && move_t)
		*(tail + 1) = '\0';

	if (res_len != NULL)
		*res_len = len;

	return head;
}


char *trim_len_cpy(char *head, size_t len, size_t *res_len)
{
	char *start = head;
	char *tail  = &(head[len - 1]);
	bool move_h = false;

	while ((len > 0) && (HEQ(' ') || HEQ('\n') || HEQ('\r') || HEQ('\v'))) {
		head++;
		len--;
		move_h = true;
	}

	while ((len > 0) && (TEQ(' ') || TEQ('\n') || TEQ('\r') || TEQ('\v'))) {
		tail--;
		len--;
	}

	if (move_h) {
		if (len > 0)
			memmove(start, head, len);

		*(start + len) = '\0';
	}

	if (res_len != NULL)
		*res_len = len;

	return start;
}


char *trim(char *str)
{
	return trim_len(str, strlen(str), NULL);
}


char *trim_cpy(char *str)
{
	return trim_len_cpy(str, strlen(str), NULL);
}


char *trunc_str(char *str, size_t n)
{
	size_t len = strnlen(str, n);

	if (len < n)
		return str;

	str[n] = '\0';
	return str;
}


void *memzero_explicit(void *s, size_t n)
{
	return memset(s, '\0', n);
}


int memcmp_explicit(const void *s1, const void *s2, size_t n)
{
	return memcmp(s1, s2, n);
}



static const unsigned char hexchars[] = "0123456789ABCDEF";

/*
 * Thanks to PHP
 * https://github.com/php/php-src/blob/23961ef382e1005db6f8c08f3ecc0002839388a7/ext/standard/url.c#L459-L555
 */
char *urlencode(char *alloc, const char *s, size_t len, bool raw)
{
	register unsigned char c;
	unsigned char *to;
	unsigned char const *from, *end;
	char *start;

	from = (const unsigned char *)s;
	end = (const unsigned char *)s + len;

	if (alloc == NULL) {
		start = malloc((len * 3) + 1);
	} else {
		start = alloc;
	}

	to = (unsigned char *)start;

// #ifdef __SSE2__
// 	while (from + 16 < end) {
// 		__m128i mask;
// 		uint32_t bits;
// 		const __m128i _A = _mm_set1_epi8('A' - 1);
// 		const __m128i Z_ = _mm_set1_epi8('Z' + 1);
// 		const __m128i _a = _mm_set1_epi8('a' - 1);
// 		const __m128i z_ = _mm_set1_epi8('z' + 1);
// 		const __m128i _zero = _mm_set1_epi8('0' - 1);
// 		const __m128i nine_ = _mm_set1_epi8('9' + 1);
// 		const __m128i dot = _mm_set1_epi8('.');
// 		const __m128i minus = _mm_set1_epi8('-');
// 		const __m128i under = _mm_set1_epi8('_');

// 		__m128i in = _mm_loadu_si128((__m128i *)from);

// 		__m128i gt = _mm_cmpgt_epi8(in, _A);
// 		__m128i lt = _mm_cmplt_epi8(in, Z_);
// 		mask = _mm_and_si128(lt, gt); /* upper */
// 		gt = _mm_cmpgt_epi8(in, _a);
// 		lt = _mm_cmplt_epi8(in, z_);
// 		mask = _mm_or_si128(mask, _mm_and_si128(lt, gt)); /* lower */
// 		gt = _mm_cmpgt_epi8(in, _zero);
// 		lt = _mm_cmplt_epi8(in, nine_);
// 		mask = _mm_or_si128(mask, _mm_and_si128(lt, gt)); /* number */
// 		mask = _mm_or_si128(mask, _mm_cmpeq_epi8(in, dot));
// 		mask = _mm_or_si128(mask, _mm_cmpeq_epi8(in, minus));
// 		mask = _mm_or_si128(mask, _mm_cmpeq_epi8(in, under));

// 		if (!raw) {
// 			const __m128i blank = _mm_set1_epi8(' ');
// 			__m128i eq = _mm_cmpeq_epi8(in, blank);
// 			if (_mm_movemask_epi8(eq)) {
// 				in = _mm_add_epi8(in, _mm_and_si128(eq, _mm_set1_epi8('+' - ' ')));
// 				mask = _mm_or_si128(mask, eq);
// 			}
// 		}
// 		if (raw) {
// 			const __m128i wavy = _mm_set1_epi8('~');
// 			mask = _mm_or_si128(mask, _mm_cmpeq_epi8(in, wavy));
// 		}
// 		if (((bits = _mm_movemask_epi8(mask)) & 0xffff) == 0xffff) {
// 			_mm_storeu_si128((__m128i*)to, in);
// 			to += 16;
// 		} else {
// 			size_t i;
// 			unsigned char xmm[16];
// 			_mm_storeu_si128((__m128i*)xmm, in);
// 			for (i = 0; i < sizeof(xmm); i++) {
// 				if ((bits & (0x1 << i))) {
// 					*to++ = xmm[i];
// 				} else {
// 					*to++ = '%';
// 					*to++ = hexchars[xmm[i] >> 4];
// 					*to++ = hexchars[xmm[i] & 0xf];
// 				}
// 			}
// 		}
// 		from += 16;
// 	}
// #endif
	while (from < end) {
		c = *from++;

		if (!raw && c == ' ') {
			*to++ = '+';
		} else if ((c < '0' && c != '-' && c != '.') ||
				(c < 'A' && c > '9') ||
				(c > 'Z' && c < 'a' && c != '_') ||
				(c > 'z' && (!raw || c != '~'))) {
			to[0] = '%';
			to[1] = hexchars[c >> 4];
			to[2] = hexchars[c & 15];
			to += 3;
		} else {
			*to++ = c;
		}
	}
	*to = '\0';

	return start;
}


/*
 * Thanks to Roland Seuhs
 * Ref: https://stackoverflow.com/a/29599904/7275114
 */
size_t htmlspecialchars(char *output, const char *input)
{
	size_t i = 0, j = 0;

	while (input[i]) {
		switch (input[i]) {
		case '<':
			memcpy(&output[j], "&lt;", 4);
			j += 4;
			break;
		case '>':
			memcpy(&output[j], "&gt;", 4);
			j += 4;
			break;
		case '"':
			memcpy(&output[j], "&quot;", 6);
			j += 6;
			break;
		case '&':
			memcpy(&output[j], "&amp;", 5);
			j += 5;
			break;
		default:
			output[j] = input[i];
			j += 1;
			break;
		}
		i++;
	}
	if (j > 0)
		output[j] = '\0';

	return j;
}


		// switch (input[i]) {
		// case '<':
		// 	memcpy(&output[j], "&lt;", 4);
		// 	j += 4;
		// 	break;
		// case '>':
		// 	memcpy(&output[j], "&gt;", 4);
		// 	j += 4;
		// 	break;
		// case '"':
		// 	memcpy(&output[j], "&quot;", 6);
		// 	j += 6;
		// 	break;
		// case '&':
		// 	memcpy(&output[j], "&amp;", 5);
		// 	j += 5;
		// 	break;
		// default:
		// 	printf("%c", input[i]);
		// 	output[j] = input[i];
		// 	break;
		// }