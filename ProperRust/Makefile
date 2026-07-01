# Thin wrapper over cargo so the commands feel like the ProperC repos.

MODULES := ownership collections linkedlist traits_generics errors threads threadpool

.PHONY: test test-solutions clean $(addprefix test_,$(MODULES))

# Run the whole suite sequentially — one module at a time.
test:
	@set -e; for m in $(MODULES); do \
		$(MAKE) --no-print-directory test_$$m; \
	done
	@echo "==== ALL MODULES PASSED ===="

# Run one module's tests, e.g. `make test_ownership`.
$(foreach m,$(MODULES),$(eval \
.PHONY: test_$(m) \
test_$(m): \
	cargo test --test test_$(m) \
))

# Run the suite against the reference solutions (sanity-checks the tests).
test-solutions:
	./check-solutions.sh

clean:
	cargo clean
	rm -rf solutions-check
