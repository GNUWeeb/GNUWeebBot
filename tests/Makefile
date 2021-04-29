#
# SPDX-License-Identifier: GPL-2.0
# 
# GNUWeebBot (GNUWeeb Telegram Bot)
# 
# https://github.com/GNUWeeb/GNUWeebBot
#



INCLUDE_DIR	+= -I$(BASE_DIR)/tests
DEP_DIRS	+= $(BASE_DEP_DIR)/src/teavpn2/tests

LD_LIBRARY_PATH	:= $(LD_LIBRARY_PATH):$(BASE_DIR)/tests:$(BASE_DIR)/src/ext/json-c/build.dir
export LD_LIBRARY_PATH

###################################################################
#
# Build Tea Test (Unit Test Framework for C project)
#
###################################################################
TEST_OBJ	:=
FWTEST_SO	:= $(BASE_DIR)/tests/libteatest.so
FWTEST_OBJ	:= $(BASE_DIR)/tests/teatest.o
OBJ_PRE_CC_WITHOUT_MAIN := $(filter-out $(BASE_DIR)/src/gwbot/main.o,$(OBJ_PRE_CC))


$(FWTEST_OBJ):
	$(S)echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	$(Q)$(CC) $(DEPFLAGS) \
		$(filter-out -Wstack-usage=$(STACK_USAGE_SIZE), $(CFLAGS)) \
		-Wno-missing-noreturn \
		-Wno-alloca \
		-c $(@:.o=.c) -o $(@)



$(FWTEST_SO): $(FWTEST_OBJ) $(OBJ_CC) $(OBJ_PRE_CC_WITHOUT_MAIN) $(SHARED_LIB)
	$(S)echo "   LD		" "$(@:$(BASE_DIR)/%=%)"
	$(Q)$(LD) $(LDFLAGS) $(^) -shared -o "$@" $(LIB_LDFLAGS)



-include $(FWTEST_OBJ:$(BASE_DIR)/%.o=$(BASE_DEP_DIR)/%.d)
##################################################################


include $(BASE_DIR)/tests/001_string/Makefile
include $(BASE_DIR)/tests/002_tstack/Makefile
include $(BASE_DIR)/tests/003_que/Makefile
include $(BASE_DIR)/tests/006_tg_event/Makefile


test: $(FWTEST_SO) $(TEST_EXEC)
	$(Q)printf '---------------------------------------------------------------\n'
	$(Q)printf '\t\t\x1b[32m All tests passed!\x1b[0m\n'
	$(Q)printf '---------------------------------------------------------------\n'


clean_test:
	$(Q)rm -vf $(FWTEST_SO) $(FWTEST_OBJ) $(TEST_EXEC:%__exec=%.test) $(TEST_OBJ)



.PHONY: test clean_test
