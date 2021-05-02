
#include <errno.h>
#include <unistd.h>
#include <gwbot/module.h>
#include <gwbot/event_handler.h>

#include "module_table.h"

static bool is_mod_initialized = false;


int gwbot_module_startup(struct gwbot_state *state)
{
	const size_t mod_c = sizeof(mod_tbl) / sizeof(*mod_tbl);

	if (likely(is_mod_initialized))
		return 0;

	prl_notice(3, "Initializing modules...");

	for (size_t i = 0; i < mod_c; i++) {
		int ret;
		const struct module_entry *mod = &mod_tbl[i];

		if (unlikely(!mod->startup))
			continue;

		prl_notice(3, "Initializing module: %s...", mod->mod_name);

		ret = mod->startup(state);
		if (unlikely(ret)) {
			pr_err("Cannot starting up module: %s " PRERF,
				mod->mod_name, PREAR(-ret));
			return ret;
		}
	}

	is_mod_initialized = true;
	return 0;
}


int gwbot_module_shutdown(struct gwbot_state *state)
{
	const size_t mod_c = sizeof(mod_tbl) / sizeof(*mod_tbl);

	if (unlikely(!is_mod_initialized))
		return 0;

	prl_notice(3, "Shutting down modules...");

	for (size_t i = 0; i < mod_c; i++) {
		int ret;
		const struct module_entry *mod = &mod_tbl[i];

		if (unlikely(!mod->shutdown))
			continue;

		prl_notice(3, "Shutting down module: %s...", mod->mod_name);

		ret = mod->shutdown(state);
		if (unlikely(ret)) {
			pr_err("Cannot shutting down module: %s " PRERF,
				mod->mod_name, PREAR(-ret));
		}
	}

	is_mod_initialized = false;
	return 0;
}


int gwbot_event_handler(const struct gwbot_thread *thread)
{
	int ret;
	struct tgev evt;
	uint64_t evt_in;
	const char *json_str;
	const size_t mod_c = sizeof(mod_tbl) / sizeof(*mod_tbl);

	memset(&evt, 0, sizeof(evt));

	if (thread->ev_from_sqe)
		json_str = thread->sqe_node->data + sizeof(uint16_t);
	else
		json_str = thread->pkt.pkt.data;

	/* TODO: Make chat_id rate limiter */

	ret = tg_event_load_str(json_str, &evt);
	if (unlikely(ret)) {
		pr_err("tg_event_load_str(): " PRERF, PREAR(-ret));
		return ret;
	}

	evt_in = evt.type;

	for (size_t i = 0; i < mod_c; i++) {
		const struct module_entry *mod = &mod_tbl[i];

		if (unlikely((mod->event_mask & evt_in) == 0))
			continue;

		if (unlikely(!mod->entry))
			continue;

		ret = mod->entry(thread, &evt);
		if (ret != -ECANCELED)
			break;
	}

	tg_event_destroy(&evt);
	return ret;
}
