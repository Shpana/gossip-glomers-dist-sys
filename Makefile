CUR_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST)))) 

warmup:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/

all-core: warmup
	cmake --build build/ --target core

format:
	find $(CUR_DIR)/core/src/ -iname '*.cpp' | xargs clang-format -i
	find $(CUR_DIR)/core/include/ -iname '*.hpp' | xargs clang-format -i