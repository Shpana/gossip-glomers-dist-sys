build-broadcast-part5:
	cmake --build $(BUILD_DIR) --target broadcast-part5

maelstrom-broadcast-part5: build-broadcast-part5
	maelstrom test \
		-w broadcast \
		--bin $(BUILD_DIR)/tasks/broadcast/part5/broadcast-part5 \
		--log-stderr \
		--node-count 25 \
		--time-limit 20 \
		--rate 100 \
		--latency 100 

.PHONY: build-broadcast-part5 maelstrom-broadcast-part5