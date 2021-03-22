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
include $(BASE_DIR)/tests/004_tcp_slot_stack/Makefile

export VG
export VGFLAGS

test:
	@+$(MAKE) --no-print-directory \
	&& $(MAKE) --no-print-directory __internal_do_test DO_TEST=1;

__internal_do_test: $(TESTS)

clean_test:
	@rm -vf $(TESTS)
