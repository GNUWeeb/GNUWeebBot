

#include <gwbot/module.h>
#include <gwbot/modules/000_debug/header.h>

const struct module_entry mod_tbl[] = {
	GWMOD_REG(000_debug, TGEV_TEXT | TGEV_PHOTO),
	GWMOD_REG_END
};
