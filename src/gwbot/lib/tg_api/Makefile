#
# SPDX-License-Identifier: GPL-2.0
#
# GNUWeebBot (GNUWeeb Telegram Bot)
#
# https://github.com/GNUWeeb/GNUWeebBot
#

DEP_DIRS	+= $(BASE_DEP_DIR)/src/gwbot/lib/tg_api

OBJ_TMP_CC	:= \
	$(BASE_DIR)/src/gwbot/lib/tg_api/get_chat_administrators.o \
	$(BASE_DIR)/src/gwbot/lib/tg_api/kick_chat_member.o \
	$(BASE_DIR)/src/gwbot/lib/tg_api/pin_chat_message.o \
	$(BASE_DIR)/src/gwbot/lib/tg_api/restrict_chat_member.o \
	$(BASE_DIR)/src/gwbot/lib/tg_api/send_message.o \
	$(BASE_DIR)/src/gwbot/lib/tg_api/unban_chat_member.o \
	$(BASE_DIR)/src/gwbot/lib/tg_api/unpin_chat_message.o \
	$(BASE_DIR)/src/gwbot/lib/tg_api/delete_message.o 

OBJ_PRE_CC	+= $(OBJ_TMP_CC)

$(OBJ_TMP_CC):
	$(Q)echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -c $(@:.o=.c) -o $(@)
