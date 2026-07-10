CUR_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST)))) 

warmup:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/

all-core: warmup
	cmake --build build/ --target core

all-echo: all-core
	cmake --build build/ --target echo

all: all-core all-echo

maelstrom-echo: all-echo
	maelstrom test -w echo --bin ./build/echo/echo --node-count 3 --time-limit 60 	

format:
	find $(CUR_DIR)core/src/ -iname '*.cpp' | xargs clang-format -i
	find $(CUR_DIR)core/include/ -iname '*.hpp' | xargs clang-format -i
