warmup:
	cmake -S . -B build/

include maelstrom/Makefile
include tasks/Makefile

tidy:
	find ./maelstrom/src/ -iname '*.cpp' | xargs clang-tidy -fix
	find ./maelstrom/include/ -iname '*.hpp' | xargs clang-tidy -fix
	find ./tasks/**/src/ -iname '*.cpp' | xargs clang-tidy -fix
	find ./tasks/**/include/ -iname '*.hpp' | xargs clang-tidy -fix

format:
	find ./maelstrom/src/ -iname '*.cpp' | xargs clang-format -i
	find ./maelstrom/include/ -iname '*.hpp' | xargs clang-format -i
	find ./tasks/**/src/ -iname '*.cpp' | xargs clang-format -i
	find ./tasks/**/include/ -iname '*.hpp' | xargs clang-format -i
