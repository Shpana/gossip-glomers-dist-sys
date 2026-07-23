build-broadcast-part1:
	cmake --build $(BUILD_DIR) --target broadcast-part1

maelstrom-broadcast-part1: build-broadcast-part1
	maelstrom test \
		-w broadcast \
		--bin $(BUILD_DIR)/tasks/broadcast/part1/broadcast-part1 \
		--log-stderr \
		--node-count 1 \
		--time-limit 20 \
		--rate 10

.PHONY: build-broadcast-part1 maelstrom-broadcast-part1