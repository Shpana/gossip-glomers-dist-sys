build-kafka-part1:
	cmake --build $(BUILD_DIR) --target kafka-part1

maelstrom-kafka-part1: build-kafka-part1
	maelstrom test \
		-w kafka \
		--bin $(BUILD_DIR)/tasks/kafka/part1/kafka-part1 \
		--node-count 1 \
		--concurrency 2n \
		--time-limit 20 \
		--rate 1000

.PHONY: build-kafka-part1 maelstrom-kafka-part1