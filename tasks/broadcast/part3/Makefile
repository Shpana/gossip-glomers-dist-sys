build-broadcast-part3:
	cmake --build $(BUILD_DIR) --target broadcast-part3

maelstrom-broadcast-part3: build-broadcast-part3
	maelstrom test \
		-w broadcast \
		--bin $(BUILD_DIR)/tasks/broadcast/part3/broadcast-part3 \
		--log-stderr \
		--node-count 5 \
		--time-limit 20 \
		--rate 10 \
		--nemesis partition

.PHONY: build-broadcast-part3 maelstrom-broadcast-part3