

#ifndef TESTS__006_TG_EVENT__006_TG_EVENT_H
#define TESTS__006_TG_EVENT__006_TG_EVENT_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <gwbot/base.h>


char *load_str_from_file(const char *file);
int tg_event_text(void);

extern uint32_t credit;
extern uint32_t total_credit;


typedef int (*test_func_t)(void);

struct list_func {
	test_func_t func;
	uint32_t credit;
};


extern struct list_func tg_event_text_list[];
extern struct list_func tg_event_photo_list[];


#define TQ_ASSERT(EXPR, CREDIT)						\
do {									\
	char fname[] = __FILE__;					\
	if ((EXPR)) {							\
		credit += (CREDIT);					\
		pr_notice(						\
			"\x1b[32mTest passed\x1b[0m: %s() in "		\
			"%s/%s line %d",				\
			__func__,					\
			basename(dirname(fname)),			\
			basename(fname),				\
			__LINE__);					\
	} else {							\
		pr_notice(						\
			"\x1b[31mTest failed\x1b[0m: %s() in "		\
			"%s/%s line %d",				\
			__func__,					\
			basename(dirname(fname)),			\
			basename(fname),				\
			__LINE__);					\
	}								\
} while (0)


/* TODO: Make core dump */
#define core_dump()
#define EXPORT_TEST(TEST) 			\
	extern uint8_t __UNIT_TEST_##TEST;	\
	uint8_t __UNIT_TEST_##TEST = 1		\


int nnstrcmp(const char *s1, const char *s2);

#endif /* #ifndef TESTS__006_TG_EVENT__006_TG_EVENT_H */
