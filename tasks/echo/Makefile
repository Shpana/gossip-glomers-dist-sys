build-echo:
	cmake --build build --target echo

maelstrom-echo: build-echo
	maelstrom test \
		-w echo 
		--bin $(CURDIR)/build/tasks/echo/echo \
		--log-stderr \
		--node-count 3 \
		--time-limit 60 	

.PHONY: build-echo maelstrom-echo