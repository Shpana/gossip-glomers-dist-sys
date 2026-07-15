build-broadcast-part4:
	cmake --build build --target broadcast-part4

maelstrom-broadcast-part4: build-broadcast-part4
	maelstrom test \
		-w broadcast \
		--bin $(CURDIR)/build/broadcast-part4/broadcast-part4 \
		--log-stderr \
		--node-count 25 \
		--time-limit 20 \
		--rate 100 \
		--latency 100 

.PHONY: build-broadcast-part4 maelstrom-broadcast-part4