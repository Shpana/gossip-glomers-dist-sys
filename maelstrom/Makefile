TESTS:= 
TESTS+=network-processor-test
TESTS+=init-test
TESTS+=echo-test
TESTS+=echo-deferred-test

BUILD_TARGETS:=
BUILD_TARGETS+=$(addprefix build-,$(TESTS))
BUILD_TARGETS+=build-maelstrom

$(BUILD_TARGETS): build-%:
	cmake --build build --target $* 

RUN_TARGETS:=$(addprefix run-,$(TESTS))

$(RUN_TARGETS): run-%: build-%
	$(CURDIR)/build/maelstrom/$*

run-tests: $(RUN_TARGETS)

.PHONY: $(BUILD_TARGETS) $(RUN_TARGETS) run-tests
