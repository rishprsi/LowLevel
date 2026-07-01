#!/bin/sh
# Oracle for the test suite: run `cargo test` against the reference
# solutions instead of the stubs in src/. If this is green but your src/
# version fails, the bug is in your implementation, not the tests.
#
# Works by copying the crate (minus target/ and .git/) into
# ./solutions-check/, overwriting each src/<module>.rs with
# solutions/<module>.rs, and running the tests there. Your src/ files are
# never touched.
set -eu

cd "$(dirname "$0")"

CHECK_DIR=solutions-check

rm -rf "$CHECK_DIR"
mkdir -p "$CHECK_DIR"

# Copy the crate, excluding build artifacts, VCS metadata, and the check
# dir itself.
for entry in Cargo.toml Cargo.lock src tests solutions; do
    [ -e "$entry" ] && cp -R "$entry" "$CHECK_DIR/"
done

# Swap every solution in over its stub.
found=0
for sol in solutions/*.rs; do
    [ -e "$sol" ] || continue
    found=1
    cp "$sol" "$CHECK_DIR/src/$(basename "$sol")"
done

if [ "$found" -eq 0 ]; then
    echo "check-solutions.sh: no files in solutions/ — nothing to check" >&2
    exit 1
fi

echo "== Running test suite against reference solutions =="
cd "$CHECK_DIR"
exec cargo test
