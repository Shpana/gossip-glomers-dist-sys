CUR_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST)))) 

warmup:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/

all-core: warmup
	cmake --build build/ --target core
all-echo: all-core
	cmake --build build/ --target 1-echo
all-unique-ids: all-core
	cmake --build build/ --target 2-unique_ids
all-broadcast-a: all-core
	cmake --build build/ --target 3a-broadcast
all-broadcast-b: all-core
	cmake --build build/ --target 3b-broadcast

maelstrom-echo: all-echo
	maelstrom test -w echo --bin ./build/1-echo/1-echo --node-count 3 --time-limit 60 	
maelstrom-unique-ids: all-unique-ids
	maelstrom test -w unique-ids --bin ./build/2-unique_ids/2-unique_ids --log-stderr --time-limit 30 --rate 1000 --node-count 3 --availability total --nemesis partition
maelstrom-broadcast-a: all-broadcast-a
	maelstrom test -w broadcast --bin ./build/3a-broadcast/3a-broadcast --log-stderr --node-count 1 --time-limit 20 --rate 10
maelstrom-broadcast-b: all-broadcast-b
	maelstrom test -w broadcast --bin ./build/3b-broadcast/3b-broadcast --log-stderr --node-count 5 --time-limit 20 --rate 10

format:
	find ./core/src/ -iname '*.cpp' | xargs clang-format -i
	find ./core/include/ -iname '*.hpp' | xargs clang-format -i

	find ./1-echo/src/ -iname '*.cpp' | xargs clang-format -i
	find ./1-echo/include/ -iname '*.hpp' | xargs clang-format -i

	find ./2-unique_ids/src/ -iname '*.cpp' | xargs clang-format -i
	find ./2-unique_ids/include/ -iname '*.hpp' | xargs clang-format -i

	find ./3a-broadcast/src/ -iname '*.cpp' | xargs clang-format -i
	find ./3a-broadcast/include/ -iname '*.hpp' | xargs clang-format -i
