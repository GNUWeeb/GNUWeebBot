

DEP_DIRS	+=  $(BASE_DEP_DIR)/tests/004_tstack
TESTS		+=	$(BASE_DIR)/tests/004_tstack/004_tstack.test__exec

$(BASE_DIR)/tests/004_tstack/004_tstack.test: $(BASE_DIR)/tests/004_tstack/004_tstack.c \
	$(TARGET_BIN) \
	$(OBJ_CC) \
	$(OBJ_PRE_CC)\
	$(BASE_DIR)/tests/004_tstack/004_tstack.c
	@echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	@$(CC) $(CFLAGS) \
	-MT "$@" -MMD -MP -MF "$(@:$(BASE_DIR)/%.test=$(BASE_DEP_DIR)/%.d)" \
	$(BASE_DIR)/tests/004_tstack/004_tstack.c \
	-o $(@) $(OBJ_PRE_CC) $(OBJ_CC) $(TEST_LIB_LDFLAGS)


-include $(BASE_DEP_DIR)/tests/004_tstack/004_tstack.d


$(BASE_DIR)/tests/004_tstack/004_tstack.test__exec: $(BASE_DIR)/tests/004_tstack/004_tstack.test
	@$(BASE_DIR)/tests/004_tstack/004_tstack.test
