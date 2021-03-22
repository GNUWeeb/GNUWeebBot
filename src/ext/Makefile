#
# SPDX-License-Identifier: GPL-2.0
# 
# GNUWeebBot (GNUWeeb Telegram Bot)
# 
# https://github.com/GNUWeeb/GNUWeebBot
#

include $(BASE_DIR)/src/ext/criterion/Makefile
include $(BASE_DIR)/src/ext/inih/Makefile
include $(BASE_DIR)/src/ext/openssl_1.1.1j/Makefile

# __execute_make_io_uring:
# 	+$(Q)cd $(BASE_DIR)/src/ext/liburing-2.0 && $(MAKE) --no-print-directory;

# __clean_io_uring:
# 	+$(Q)cd $(BASE_DIR)/src/ext/liburing-2.0 && $(MAKE) --no-print-directory clean;

# LIB_LDFLAGS	+= -L$(BASE_DIR)/src/ext/liburing-2.0/src -luring
# SHARED_LIB	+= $(BASE_DIR)/src/ext/liburing-2.0/src/liburing.so.2.0.0

# $(BASE_DIR)/src/ext/liburing-2.0/src/liburing.so.2.0.0: __execute_make_io_uring

# clean_ext: __clean_io_uring

build-json-c: $(BASE_DIR)/lib
	mkdir json-c-build
	cd json-c-build
	cmake ../json-c
