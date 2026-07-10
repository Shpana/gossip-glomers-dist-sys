CUR_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST)))) 

warmup:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/

all-core: warmup
	cmake --build build/ --target core

all-echo: all-core
	cmake --build build/ --target echo

all-unique-ids: all-core
	cmake --build build/ --target unique_ids

all: all-core all-echo all-unique-ids

maelstrom-echo: all-echo
	maelstrom test -w echo --bin ./build/echo/echo --node-count 3 --time-limit 60 	

maelstrom-unique-ids: all-unique-ids
	maelstrom test -w unique-ids --bin ./build/unique_ids/unique_ids --log-stderr --time-limit 30 --rate 1000 --node-count 3 --availability total --nemesis partition

format:
	find $(CUR_DIR)core/src/ -iname '*.cpp' | xargs clang-format -i
	find $(CUR_DIR)core/include/ -iname '*.hpp' | xargs clang-format -i
