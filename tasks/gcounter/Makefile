build-gcounter:
	cmake --build $(BUILD_DIR) --target gcounter

maelstrom-gcounter: build-gcounter
	maelstrom test \
		-w g-counter \
		--bin $(BUILD_DIR)/tasks/gcounter/gcounter \
		--node-count 50 \
		--rate 100 \
		--time-limit 20 \
		--nemesis partition

.PHONY: build-gcounter maelstrom-gcounter