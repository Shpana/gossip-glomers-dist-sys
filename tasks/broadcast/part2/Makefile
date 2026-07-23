build-broadcast-part2:
	cmake --build $(BUILD_DIR) --target broadcast-part2

maelstrom-broadcast-part2: build-broadcast-part2
	maelstrom test \
		-w broadcast \
		--bin $(BUILD_DIR)/tasks/broadcast/part2/broadcast-part2 \
		--log-stderr \
		--node-count 5 \
		--time-limit 20 \
		--rate 10

.PHONY: build-broadcast-part2 maelstrom-broadcast-part2