

DEP_DIRS	+=  $(BASE_DEP_DIR)/tests/006_tg_event
TESTS		+=	$(BASE_DIR)/tests/006_tg_event/006_tg_event.test__exec


__006_TG_EVENT_DEPS = \
	$(BASE_DIR)/tests/006_tg_event/006_tg_event.o \
	$(BASE_DIR)/tests/006_tg_event/tg_event_text.o


$(BASE_DIR)/tests/006_tg_event/006_tg_event.o: $(BASE_DIR)/tests/006_tg_event/006_tg_event.c
	@echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	@$(CC) $(CFLAGS) \
		-MT "$@" -MMD -MP -MF "$(@:$(BASE_DIR)/%.test=$(BASE_DEP_DIR)/%.d)" \
		-DMY_BASEPATH=\"$(BASE_DIR)/tests/006_tg_event\" \
		$(@:%.o=%.c) \
		-c -o $(@)


$(BASE_DIR)/tests/006_tg_event/tg_event_text.o: $(BASE_DIR)/tests/006_tg_event/tg_event_text.c
	@echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	@$(CC) $(CFLAGS) \
		-MT "$@" -MMD -MP -MF "$(@:$(BASE_DIR)/%.test=$(BASE_DEP_DIR)/%.d)" \
		-DMY_BASEPATH=\"$(BASE_DIR)/tests/006_tg_event\" \
		$(@:%.o=%.c) \
		-c -o $(@)


$(BASE_DIR)/tests/006_tg_event/006_tg_event.test: $(TARGET_BIN) $(OBJ_CC) $(OBJ_PRE_CC) $(__006_TG_EVENT_DEPS)
	@echo "   CC		" "$(@:$(BASE_DIR)/%=%)"
	@$(CC) $(CFLAGS) \
		-MT "$@" -MMD -MP -MF "$(@:$(BASE_DIR)/%.test=$(BASE_DEP_DIR)/%.d)" \
		$(__006_TG_EVENT_DEPS) \
		-o $(@) $(OBJ_PRE_CC) $(OBJ_CC) $(LIB_LDFLAGS)


-include $(BASE_DEP_DIR)/tests/006_tg_event/tg_event_text.d
-include $(BASE_DEP_DIR)/tests/006_tg_event/006_tg_event.d

TESTS_OBJ += $(__006_TG_EVENT_DEPS)

$(BASE_DIR)/tests/006_tg_event/006_tg_event.test__exec: $(BASE_DIR)/tests/006_tg_event/006_tg_event.test
	@$(BASE_DIR)/tests/006_tg_event/006_tg_event.test
