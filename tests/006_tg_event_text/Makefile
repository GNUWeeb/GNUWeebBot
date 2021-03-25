

DEP_DIRS	+=  $(BASE_DEP_DIR)/tests/006_tg_event_text
TESTS		+=	$(BASE_DIR)/tests/006_tg_event_text/006_tg_event_text.test__exec

$(BASE_DIR)/tests/006_tg_event_text/006_tg_event_text.test: $(BASE_DIR)/tests/006_tg_event_text/006_tg_event_text.c \
	$(TARGET_BIN) \
	$(OBJ_CC) \
	$(OBJ_PRE_CC)\
	$(BASE_DIR)/tests/006_tg_event_text/006_tg_event_text.c
	@echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	@$(CC) $(CFLAGS) \
	-MT "$@" -MMD -MP -MF "$(@:$(BASE_DIR)/%.test=$(BASE_DEP_DIR)/%.d)" \
	$(BASE_DIR)/tests/006_tg_event_text/006_tg_event_text.c \
	-o $(@) $(OBJ_PRE_CC) $(OBJ_CC) $(TEST_LIB_LDFLAGS)


-include $(BASE_DEP_DIR)/tests/006_tg_event_text/006_tg_event_text.d


$(BASE_DIR)/tests/006_tg_event_text/006_tg_event_text.test__exec: $(BASE_DIR)/tests/006_tg_event_text/006_tg_event_text.test
	@$(BASE_DIR)/tests/006_tg_event_text/006_tg_event_text.test
