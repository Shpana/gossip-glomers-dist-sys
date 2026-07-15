warmup:
	cmake -S . -B build/

all-maelstrom: warmup
	cmake --build build/ --target maelstrom
all-echo: all-maelstrom
	cmake --build build/ --target 1-echo
all-unique-ids: all-maelstrom
	cmake --build build/ --target 2-unique_ids
all-broadcast-a: all-maelstrom
	cmake --build build/ --target 3a-broadcast
all-broadcast-b: all-maelstrom
	cmake --build build/ --target 3b-broadcast
all-broadcast-c: all-maelstrom
	cmake --build build/ --target 3c-broadcast
all-broadcast-d: all-maelstrom
	cmake --build build/ --target 3d-broadcast
all-broadcast-e: all-maelstrom
	cmake --build build/ --target 3e-broadcast
all-gcounter: all-maelstrom
	cmake --build build/ --target 4-gcounter 

maelstrom-echo: all-echo
	maelstrom test -w echo --bin ./build/1-echo/1-echo --node-count 3 --time-limit 60 	
maelstrom-unique-ids: all-unique-ids
	maelstrom test -w unique-ids --bin ./build/2-unique_ids/2-unique_ids --log-stderr --time-limit 30 --rate 1000 --node-count 3 --availability total --nemesis partition
maelstrom-broadcast-a: all-broadcast-a
	maelstrom test -w broadcast --bin ./build/3a-broadcast/3a-broadcast --log-stderr --node-count 1 --time-limit 20 --rate 10
maelstrom-broadcast-b: all-broadcast-b
	maelstrom test -w broadcast --bin ./build/3b-broadcast/3b-broadcast --log-stderr --node-count 5 --time-limit 20 --rate 10
maelstrom-broadcast-c: all-broadcast-c
	maelstrom test -w broadcast --bin ./build/3c-broadcast/3c-broadcast --log-stderr --node-count 5 --time-limit 20 --rate 10 --nemesis partition
maelstrom-broadcast-d: all-broadcast-d
	maelstrom test -w broadcast --bin ./build/3d-broadcast/3d-broadcast --log-stderr --node-count 25 --time-limit 20 --rate 100 --latency 100 
maelstrom-broadcast-e: all-broadcast-e
	maelstrom test -w broadcast --bin ./build/3e-broadcast/3e-broadcast --log-stderr --node-count 25 --time-limit 20 --rate 100 --latency 100 

format:
	find ./maelstrom/src/ -iname '*.cpp' | xargs clang-format -i
	find ./maelstrom/include/ -iname '*.hpp' | xargs clang-format -i
	find ./tasks/**/src/ -iname '*.cpp' | xargs clang-format -i
	find ./tasks/**/include/ -iname '*.hpp' | xargs clang-format -i
