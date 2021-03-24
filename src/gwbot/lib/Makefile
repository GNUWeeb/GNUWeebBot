#
# SPDX-License-Identifier: GPL-2.0
# 
# GNUWeebBot (GNUWeeb Telegram Bot)
# 
# https://github.com/GNUWeeb/GNUWeebBot
#

DEP_DIRS	+= $(BASE_DEP_DIR)/src/gwbot/lib

include $(BASE_DIR)/src/gwbot/lib/tg_api/Makefile

OBJ_TMP_CC	:= \
	$(BASE_DIR)/src/gwbot/lib/arena.o  \
	$(BASE_DIR)/src/gwbot/lib/shell.o  \
	$(BASE_DIR)/src/gwbot/lib/string.o \
	$(BASE_DIR)/src/gwbot/lib/tg_api.o \
	$(BASE_DIR)/src/gwbot/lib/tg_event.o

OBJ_PRE_CC	+= $(OBJ_TMP_CC)

$(OBJ_TMP_CC):
	$(Q)echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -c $(@:.o=.c) -o $(@)
