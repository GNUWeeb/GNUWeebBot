

#include <gwbot/module.h>
#include <gwbot/modules/000_debug/header.h>
#include <gwbot/modules/001_standard/header.h>
#include <gwbot/modules/002_translate/header.h>
#include <gwbot/modules/003_admin/header.h>

const struct module_entry mod_tbl[] = {
	GWMOD_REG(000_debug, TGEV_TEXT | TGEV_PHOTO),
	GWMOD_REG(001_standard, TGEV_TEXT | TGEV_PHOTO),
	GWMOD_REG(002_translate, TGEV_TEXT | TGEV_PHOTO),
	GWMOD_REG(003_admin, TGEV_TEXT | TGEV_PHOTO),
	GWMOD_REG_END
};
