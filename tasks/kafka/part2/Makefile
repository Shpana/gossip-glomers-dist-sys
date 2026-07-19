build-kafka-part2:
	cmake --build build --target kafka-part2

maelstrom-kafka-part2: build-kafka-part2
	maelstrom test \
		-w kafka \
		--bin $(CURDIR)/build/tasks/kafka/part2/kafka-part2 \
		--node-count 2 \
		--concurrency 2n \
		--time-limit 20 \
		--rate 1000

.PHONY: build-kafka-part2 maelstrom-kafka-part2