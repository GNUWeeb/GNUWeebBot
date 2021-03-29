

DEP_DIRS	+=  $(BASE_DEP_DIR)/tests/005_sqe
TESTS		+=	$(BASE_DIR)/tests/005_sqe/005_sqe.test__exec

$(BASE_DIR)/tests/005_sqe/005_sqe.test: $(BASE_DIR)/tests/005_sqe/005_sqe.c \
	$(TARGET_BIN) \
	$(OBJ_CC) \
	$(OBJ_PRE_CC)\
	$(BASE_DIR)/tests/005_sqe/005_sqe.c
	@echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	@$(CC) $(CFLAGS) \
	-MT "$@" -MMD -MP -MF "$(@:$(BASE_DIR)/%.test=$(BASE_DEP_DIR)/%.d)" \
	$(BASE_DIR)/tests/005_sqe/005_sqe.c \
	-o $(@) $(OBJ_PRE_CC) $(OBJ_CC) $(TEST_LIB_LDFLAGS)


-include $(BASE_DEP_DIR)/tests/005_sqe/005_sqe.d


$(BASE_DIR)/tests/005_sqe/005_sqe.test__exec: $(BASE_DIR)/tests/005_sqe/005_sqe.test
	@$(BASE_DIR)/tests/005_sqe/005_sqe.test
