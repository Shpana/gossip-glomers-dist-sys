BUILD_DIR:=$(CURDIR)/build

warmup:
	cmake -S . -B $(BUILD_DIR)

include maelstrom/Makefile
include tasks/Makefile

tidy:
	find ./maelstrom/src/ -iname '*.cpp' | xargs clang-tidy -p=$(BUILD_DIR) -fix-errors
	find ./maelstrom/include/ -iname '*.hpp' | xargs clang-tidy -p=$(BUILD_DIR) -fix-errors
	find ./maelstrom/tests/ -iname '*.cpp' | xargs clang-tidy -p=$(BUILD_DIR) -fix-errors
	find ./tasks/**/src/ -iname '*.cpp' | xargs clang-tidy -p=$(BUILD_DIR) -fix-errors
	find ./tasks/**/include/ -iname '*.hpp' | xargs clang-tidy -p=$(BUILD_DIR) -fix-errors

format:
	find ./maelstrom/src/ -iname '*.cpp' | xargs clang-format -i
	find ./maelstrom/include/ -iname '*.hpp' | xargs clang-format -i
	find ./maelstrom/tests/ -iname '*.cpp' | xargs clang-format -i
	find ./tasks/**/src/ -iname '*.cpp' | xargs clang-format -i
	find ./tasks/**/include/ -iname '*.hpp' | xargs clang-format -i
