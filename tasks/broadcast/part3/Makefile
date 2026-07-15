build-broadcast-part3:
	cmake --build build --target broadcast-part3

maelstrom-broadcast-part3: build-broadcast-part3
	maelstrom test \
		-w broadcast \
		--bin $(CURDIR)/build/broadcast-part3/broadcast-part3 \
		--log-stderr \
		--node-count 5 \
		--time-limit 20 \
		--rate 10 \
		--nemesis partition

.PHONY: build-broadcast-part1 maelstrom-broadcast-part1