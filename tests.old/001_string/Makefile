

DEP_DIRS	+=  $(BASE_DEP_DIR)/tests/001_string
TESTS		+=	$(BASE_DIR)/tests/001_string/001_string.test__exec

$(BASE_DIR)/tests/001_string/001_string.test: $(BASE_DIR)/tests/001_string/001_string.c \
	$(TARGET_BIN) \
	$(OBJ_CC) \
	$(OBJ_PRE_CC)\
	$(BASE_DIR)/tests/001_string/001_string.c
	@echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	@$(CC) $(CFLAGS) \
	-MT "$@" -MMD -MP -MF "$(@:$(BASE_DIR)/%.test=$(BASE_DEP_DIR)/%.d)" \
	$(BASE_DIR)/tests/001_string/001_string.c \
	-o $(@) $(OBJ_PRE_CC) $(OBJ_CC) $(TEST_LIB_LDFLAGS)


-include $(BASE_DEP_DIR)/tests/001_string/001_string.d


$(BASE_DIR)/tests/001_string/001_string.test__exec: $(BASE_DIR)/tests/001_string/001_string.test
	@$(BASE_DIR)/tests/001_string/001_string.test
