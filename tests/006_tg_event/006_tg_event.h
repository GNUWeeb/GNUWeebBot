

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


/* TODO: Make core dump */
#define core_dump()

#endif /* #ifndef TESTS__006_TG_EVENT__006_TG_EVENT_H */
