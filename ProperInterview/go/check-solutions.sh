#!/bin/sh
# Oracle: run the Go test suites against the reference solutions instead of
# your stubs. Copies the module into ./solutions-check/, swaps each
# solutions/<drill>.go in over <drill>/<drill>.go, and runs `go test -race`
# there. Your files are never touched.
set -eu

cd "$(dirname "$0")"

CHECK_DIR=solutions-check

rm -rf "$CHECK_DIR"
mkdir -p "$CHECK_DIR"

cp go.mod "$CHECK_DIR/"
for d in dupes workerpool counter; do
    mkdir -p "$CHECK_DIR/$d"
    cp "$d"/*.go "$CHECK_DIR/$d/"
    if [ -f "solutions/$d/$d.go" ]; then
        cp "solutions/$d/$d.go" "$CHECK_DIR/$d/$d.go"
    else
        echo "check-solutions.sh: missing solutions/$d/$d.go" >&2
        exit 1
    fi
done

echo "== Running Go test suite against reference solutions (with -race) =="
cd "$CHECK_DIR"
exec go test -race ./...
