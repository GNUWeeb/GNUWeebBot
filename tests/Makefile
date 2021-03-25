#
# SPDX-License-Identifier: GPL-2.0
# 
# GNUWeebBot (GNUWeeb Telegram Bot)
# 
# https://github.com/GNUWeeb/GNUWeebBot
#
# Makefile for unit test
#

include $(BASE_DIR)/tests/001_string/Makefile
include $(BASE_DIR)/tests/004_tstack/Makefile
include $(BASE_DIR)/tests/005_sqe/Makefile
include $(BASE_DIR)/tests/006_tg_event/Makefile

export VG
export VGFLAGS
export LD_LIBRARY_PATH=$(BASE_DIR)/src/ext/json-c/build.dir:$(BASE_DIR)/src/ext/criterion/lib
export LD_PRELOAD=$(BASE_DIR)/src/ext/json-c/build.dir/libjson-c.so.5

test:
	@+$(MAKE) --no-print-directory \
	&& $(MAKE) --no-print-directory __internal_do_test DO_TEST=1;

__internal_do_test: $(TESTS)

clean_test:
	@rm -vf $(TESTS) $(TESTS_OBJ)
