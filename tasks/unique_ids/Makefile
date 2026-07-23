build-unique-ids:
	cmake --build $(BUILD_DIR) --target unique-ids

maelstrom-unique-ids: build-unique-ids
	maelstrom test \
		-w unique-ids \
		--bin $(BUILD_DIR)/tasks/unique_ids/unique-ids \
		--log-stderr \
		--time-limit 30 \
		--rate 1000 \
		--node-count 3 \
		--availability total \
		--nemesis partition

.PHONY: build-unique-ids maelstrom-unique-ids