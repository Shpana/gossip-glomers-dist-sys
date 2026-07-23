build-echo:
	cmake --build $(BUILD_DIR) --target echo

maelstrom-echo: build-echo
	maelstrom test \
		-w echo \
		--bin $(BUILD_DIR)/tasks/echo/echo \
		--log-stderr \
		--node-count 3 \
		--time-limit 60 	

.PHONY: build-echo maelstrom-echo