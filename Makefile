#
# SPDX-License-Identifier: GPL-2.0
# 
# GNUWeebBot (GNUWeeb Telegram Bot)
# 
# https://github.com/GNUWeeb/GNUWeebBot
#
# Main Makefile
#

VERSION = 0
PATCHLEVEL = 0
SUBLEVEL = 1
EXTRAVERSION = -rc1
NAME = Useless Servant
PACKAGE_NAME = gwbot-$(VERSION).$(PATCHLEVEL).$(SUBLEVEL)$(EXTRAVERSION)

CC	:= clang
CXX	:= clang++
LD	:= $(CXX)
VG	:= valgrind

BASE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
BASE_DIR := $(strip $(patsubst %/, %, $(BASE_DIR)))
BASE_DEP_DIR := $(BASE_DIR)/.deps
MAKEFILE_FILE := $(lastword $(MAKEFILE_LIST))

TARGET_BIN := gwbot
LICENSES_DIR := $(BASE_DIR)/LICENSES
GWBOT_LICENSE_FILE := $(BASE_DIR)/LICENSE
GWBOT_README_FILE := $(BASE_DIR)/README.md
GWBOT_CONFIG_DIR := $(BASE_DIR)/config
GWBOT_DATA_DIR := $(BASE_DIR)/data

CC_BUILTIN_CONSTANTS := $(shell $(CC) -dM -E - < /dev/null)
CXX_BUILTIN_CONSTANTS := $(shell $(CXX) -dM -E - < /dev/null)

ifeq (,$(findstring __GNUC__,$(CXX_BUILTIN_CONSTANTS)))
$(error I want __GNUC__!)
endif

ifeq ($(VERBOSE),1)
	V = 1
endif

ifndef V
	V = 0
endif

ifneq ($(DO_TEST),1)
	ifneq (,$(findstring __GNUC__,$(CC_BUILTIN_CONSTANTS)))
		ifneq (,$(findstring __clang__,$(CC_BUILTIN_CONSTANTS)))
			# Clang
			WARN_FLAGS	:= \
				-Wall \
				-Wextra \
				-Wpadded \
				-Weverything \
				-Wno-disabled-macro-expansion \
				-Wno-unused-macros \
				-Wno-language-extension-token \
				-Wno-covered-switch-default
		else
			# Pure GCC
			WARN_FLAGS	:= \
				-Wall \
				-Wextra \
				-Wstrict-aliasing=3 \
				-Wformat \
				-Wformat-security \
				-Wformat-signedness \
				-Wsequence-point \
				-Wunsafe-loop-optimizations \
				-Wstack-usage=2097152
		endif
	else
	$(error I want __GNUC__!)
	endif
endif

DEPFLAGS	 = -MT "$@" -MMD -MP -MF "$(@:$(BASE_DIR)/%.o=$(BASE_DEP_DIR)/%.d)"
LIB_LDFLAGS	:= -lpthread -lcurl
LDFLAGS		:= -fPIE -fpie
CFLAGS		:= -fPIE -fpie
CXXFLAGS	:= -fPIE -fpie -std=c++2a
VGFLAGS		:= \
	--leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	--track-fds=yes \
	--error-exitcode=99 \
	--exit-on-first-error=yes -s

ifndef DEFAULT_OPTIMIZATION
	DEFAULT_OPTIMIZATION = -O0
endif

# Quite
ifeq ($(V),1)
	Q=
else
	Q=@
endif

# CCXXFLAGS is a flag that applies to CFLAGS and CXXFLAGS
CCXXFLAGS := \
	$(WARN_FLAGS) \
	-fstrict-aliasing \
	-fstack-protector-strong \
	-pedantic-errors \
	-D_GNU_SOURCE \
	-DVERSION=$(VERSION) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-DSUBLEVEL=$(SUBLEVEL) \
	-DEXTRAVERSION=\"$(EXTRAVERSION)\"

ifeq ($(RELEASE_MODE),1)
	REL := --- Build release mode
	LDFLAGS		+= $(LDFLAGS) -O3
	CCXXFLAGS	+= -O3 -DNDEBUG

	ifndef NOTICE_MAX_LEVEL
		NOTICE_MAX_LEVEL = 4
	endif

	ifndef NOTICE_ALWAYS_EXEC
		NOTICE_ALWAYS_EXEC = 0
	endif

	ifndef DEFAULT_NOTICE_LEVEL
		DEFAULT_NOTICE_LEVEL = 4
	endif
else
	REL := --- Build debug mode
	LDFLAGS		+= $(DEFAULT_OPTIMIZATION)
	CCXXFLAGS	+= \
		$(DEFAULT_OPTIMIZATION) \
		-ggdb3 \
		-grecord-gcc-switches \
		-DTEAVPN_DEBUG

	ifndef NOTICE_MAX_LEVEL
		NOTICE_MAX_LEVEL = 10
	endif

	ifndef NOTICE_ALWAYS_EXEC
		NOTICE_ALWAYS_EXEC = 1
	endif

	ifndef DEFAULT_NOTICE_LEVEL
		DEFAULT_NOTICE_LEVEL = 5
	endif
endif

CCXXFLAGS := \
	$(CCXXFLAGS) \
	-DNOTICE_MAX_LEVEL="$(NOTICE_MAX_LEVEL)" \
	-DNOTICE_ALWAYS_EXEC="$(NOTICE_ALWAYS_EXEC)" \
	-DDEFAULT_NOTICE_LEVEL="$(DEFAULT_NOTICE_LEVEL)"

ifeq ($(OS),Windows_NT)
	CCXXFLAGS += -DWIN32
	ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
		CCXXFLAGS += -DAMD64
	else
		ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
			CCXXFLAGS += -DAMD64
		endif
		ifeq ($(PROCESSOR_ARCHITECTURE),x86)
			CCXXFLAGS += -DIA32
		endif
	endif
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CCXXFLAGS += -DLINUX
	endif
	ifeq ($(UNAME_S),Darwin)
		CCXXFLAGS += -DOSX
	endif

	UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P),x86_64)
		CCXXFLAGS += -DAMD64
	endif
	ifneq ($(filter %86,$(UNAME_P)),)
		CCXXFLAGS += -DIA32
	endif
	ifneq ($(filter arm%,$(UNAME_P)),)
		CCXXFLAGS += -DARM
	endif
endif

#######################################
# Force these to be a simple variable
TESTS		:=
OBJ_CC		:=
OBJ_PRE_CC	:=
OBJ_TMP_CC	:=
CFLAGS_TMP	:=
SHARED_LIB	:=
#######################################

all: $(TARGET_BIN)
	$(Q)echo $(REL)

include $(BASE_DIR)/src/gwbot/Makefile
include $(BASE_DIR)/src/ext/Makefile

CFLAGS		:= $(INCLUDE_DIR) $(CFLAGS) $(CCXXFLAGS)
CXXFLAGS	:= $(INCLUDE_DIR) $(CXXFLAGS) $(CCXXFLAGS)

include $(BASE_DIR)/tests/Makefile

$(TARGET_BIN): $(OBJ_CC) $(OBJ_PRE_CC) $(SHARED_LIB)
	$(Q)echo "   LD		" "$(@)"
	$(Q)$(LD) $(LDFLAGS) $(OBJ_CC) $(OBJ_PRE_CC) -o "$@" $(LIB_LDFLAGS)
	$(Q)chmod a+x gwbot || true

$(DEP_DIRS):
	$(Q)echo "   MKDIR	" "$(@:$(BASE_DIR)/%=%)"
	$(Q)mkdir -p $(@)

$(OBJ_CC): $(MAKEFILE_FILE) | $(DEP_DIRS)
	$(Q)echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -c $(@:.o=.c) -o $(@)

$(OBJ_PRE_CC): $(MAKEFILE_FILE) | $(DEP_DIRS)

-include $(OBJ_CC:$(BASE_DIR)/%.o=$(BASE_DEP_DIR)/%.d)
-include $(OBJ_PRE_CC:$(BASE_DIR)/%.o=$(BASE_DEP_DIR)/%.d)

clean: clean_test
	$(Q)rm -rfv $(DEP_DIRS) $(OBJ_CC) $(OBJ_PRE_CC) $(TARGET_BIN)

clean_all: clean clean_ext

run_vg: $(TARGET_BIN)
	$(VG) $(VGFLAGS) ./$(TARGET_BIN) -c config_1.ini
