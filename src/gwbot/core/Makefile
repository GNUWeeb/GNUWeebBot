#
# SPDX-License-Identifier: GPL-2.0
# 
# GNUWeebBot (GNUWeeb Telegram Bot)
# 
# https://github.com/GNUWeeb/GNUWeebBot
#

DEP_DIRS	+= $(BASE_DEP_DIR)/src/gwbot/core

OBJ_TMP_CC	:= \
	$(BASE_DIR)/src/gwbot/core/event_handler.o \
	$(BASE_DIR)/src/gwbot/core/event_logger.o \
	$(BASE_DIR)/src/gwbot/core/server.o

OBJ_PRE_CC	+= $(OBJ_TMP_CC)

$(OBJ_TMP_CC):
	$(Q)echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -c $(@:.o=.c) -o $(@)
