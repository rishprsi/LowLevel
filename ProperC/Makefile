CC  = cc

# Sanitizers are on by default (ASan + UBSan), matching the project's habit.
# Override if a constrained environment can't run a given sanitizer, e.g.:
#   make test SAN=undefined     # UBSan only
#   make test SAN=              # no sanitizers
SAN     ?= address,undefined
SANFLAG  = $(if $(strip $(SAN)),-fsanitize=$(SAN),)

CFLAGS  = -std=c17 -Wall -Wextra -Werror -g $(SANFLAG) -Isrc -Itests $(EXTRA_CFLAGS)

SRC_DIR   = src
TEST_DIR  = tests
SOL_DIR   = solutions
BUILD_DIR = build

# One module = src/<m>.c + tests/test_<m>.c (+ optional solutions/<m>.c)
MODULES = pointers strings memory bits structs funcptr linkedlist safety ub

TEST_BINS = $(addprefix $(BUILD_DIR)/test_,$(MODULES))
SOL_BINS  = $(addprefix $(BUILD_DIR)/sol_,$(MODULES))

.PHONY: all test test-solutions list clean $(addprefix test_,$(MODULES)) $(addprefix sol_,$(MODULES))

## Build every module's test binary (against YOUR src/ implementations), without running.
all: $(TEST_BINS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# test binary <- your implementation
$(BUILD_DIR)/test_%: $(TEST_DIR)/test_%.c $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# solution binary <- reference implementation (git-ignored)
$(BUILD_DIR)/sol_%: $(TEST_DIR)/test_%.c $(SOL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

## Run all modules sequentially — each step builds and runs ONE binary only.
test:
	@set -e; for m in $(MODULES); do \
		$(MAKE) --no-print-directory test_$$m; \
	done
	@echo "==== ALL MODULES PASSED ===="

## Sanity-check the suite itself against the reference solutions, one module at a time.
test-solutions:
	@set -e; for m in $(MODULES); do \
		$(MAKE) --no-print-directory sol_$$m; \
	done
	@echo "==== ALL SOLUTION MODULES PASSED ===="

## Per-module: `make test_strings` builds+runs ONLY that module's binary.
define RUN_TEST
.PHONY: test_$(1)
test_$(1): $(BUILD_DIR)/test_$(1)
	./$(BUILD_DIR)/test_$(1)
endef
$(foreach m,$(MODULES),$(eval $(call RUN_TEST,$(m))))

## Per-module against solution: `make sol_strings` builds+runs ONLY that module.
define RUN_SOL
.PHONY: sol_$(1)
sol_$(1): $(BUILD_DIR)/sol_$(1)
	./$(BUILD_DIR)/sol_$(1)
endef
$(foreach m,$(MODULES),$(eval $(call RUN_SOL,$(m))))

list:
	@echo "Modules: $(MODULES)"

clean:
	rm -rf $(BUILD_DIR)
