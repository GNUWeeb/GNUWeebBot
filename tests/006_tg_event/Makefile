#
# SPDX-License-Identifier: GPL-2.0
# 
# GNUWeebBot (GNUWeeb Telegram Bot)
# 
# https://github.com/GNUWeeb/GNUWeebBot
#


DEP_DIRS	+= $(BASE_DEP_DIR)/tests/006_tg_event
TEST_TVAR	:= $(BASE_DIR)/tests/006_tg_event/006_tg_event
TEST_EXEC	+= $(TEST_TVAR)__exec
TEST_TVAR_OBJ	:= $(BASE_DIR)/tests/006_tg_event/006_tg_event.o
TEST_OBJ	+= $(TEST_TVAR_OBJ)


$(TEST_TVAR_OBJ):
	$(S)echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	$(Q)$(CC) -DTHIS_TEST_BASEPATH="\"$(BASE_DIR)/tests/006_tg_event\"" \
		$(DEPFLAGS) $(CFLAGS) -c $(@:.o=.c) -o $(@)



$(TEST_TVAR).test: $(TEST_TVAR_OBJ) $(FWTEST_SO)
	$(S)echo "   LD		" "$(@)"
	$(Q)$(LD) $(LDFLAGS) -o "$@" $(^) $(LIB_LDFLAGS)



-include $(TEST_TVAR_OBJ:$(BASE_DIR)/%.o=$(BASE_DEP_DIR)/%.d)



$(TEST_TVAR)__exec: $(TEST_TVAR).test
	$(<) $(shell which $(VG)) $(VGFLAGS)



.PHONY: $(TEST_TVAR)
