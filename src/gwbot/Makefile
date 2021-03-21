#
# SPDX-License-Identifier: GPL-2.0
# 
# GNUWeebBot (GNUWeeb Telegram Bot)
# 
# https://github.com/GNUWeeb/GNUWeebBot
#

INCLUDE_DIR += -I$(BASE_DIR)/src/gwbot/include
DEP_DIRS	+= $(BASE_DEP_DIR)/src/gwbot

OBJ_TMP_CC	:= \
	$(BASE_DIR)/src/gwbot/print.o

ifneq ($(DO_TEST),1)
	OBJ_TMP_CC += $(BASE_DIR)/src/gwbot/main.o
endif

OBJ_PRE_CC	+= $(OBJ_TMP_CC)

$(OBJ_TMP_CC):
	$(Q)echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -c $(@:.o=.c) -o $(@)
