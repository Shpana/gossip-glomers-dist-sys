build-gcounter:
	cmake --build build --target gcounter

maelstrom-gcounter: build-gcounter
	maelstrom test \
		-w g-counter \
		--bin $(CURDIR)/build/tasks/gcounter/gcounter \
		--node-count 20 \
		--rate 100 \
		--time-limit 20 \
		--nemesis partition

.PHONY: build-gcounter maelstrom-gcounter