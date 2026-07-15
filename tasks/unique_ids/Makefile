build-unique-ids:
	cmake --build build --target unique-ids

maelstrom-unique-ids: build-unique-ids
	maelstrom test \
		-w unique-ids \
		--bin $(CURDIR)/build/tasks/unique-ids/unique-ids \
		--log-stderr \
		--time-limit 30 \
		--rate 1000 \
		--node-count 3 \
		--availability total \
		--nemesis partition

.PHONY: build-unique-ids maelstrom-unique-ids