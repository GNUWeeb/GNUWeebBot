#
# SPDX-License-Identifier: GPL-2.0
# 
# GNUWeebBot (GNUWeeb Telegram Bot)
# 
# https://github.com/GNUWeeb/GNUWeebBot
#


INCLUDE_DIR	+= -I$(BASE_DIR)/src -I$(BASE_DIR)/src/gwbot/include
DEP_DIRS	+= $(BASE_DEP_DIR)/src/gwbot


include $(BASE_DIR)/src/gwbot/lib/Makefile
include $(BASE_DIR)/src/gwbot/modules/Makefile


OBJ_TMP_CC	:= \
	$(BASE_DIR)/src/gwbot/argv.o \
	$(BASE_DIR)/src/gwbot/config.o \
	$(BASE_DIR)/src/gwbot/event_handler.o \
	$(BASE_DIR)/src/gwbot/help.o \
	$(BASE_DIR)/src/gwbot/gwbot.o \
	$(BASE_DIR)/src/gwbot/print.o \
	$(BASE_DIR)/src/gwbot/main.o


OBJ_PRE_CC	+= $(OBJ_TMP_CC)


$(OBJ_TMP_CC):
	$(S)echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -c $(@:.o=.c) -o $(@)
