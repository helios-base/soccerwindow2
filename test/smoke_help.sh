#!/bin/sh
#
# Smoke test: start soccerwindow2 with --help on the offscreen platform.
#
# Usage: smoke_help.sh [/path/to/soccerwindow2]
#
# The program path is given by the first argument or by the environment
# variable SOCCERWINDOW2.
#
# The test passes when:
#   - "Usage:" is printed to the standard output,
#   - the options to select the transport (--transport, --ws-path) are
#     printed, and
#   - no "QObject::connect" warning is printed to the standard error
#     (MainWindow is created before the command line is parsed, so the
#     signal/slot connections made at that time are checked).
#
# HOME, XDG_CONFIG_HOME and XDG_RUNTIME_DIR are set to a temporary directory
# so that the user's ~/.soccerwindow2 is neither read nor modified.
#
# Exit status: 0 = pass, 1 = fail, 99 = error (Automake convention).

program="${1:-${SOCCERWINDOW2:-}}"
if [ -z "$program" ]; then
    echo "usage: $0 /path/to/soccerwindow2 (or set SOCCERWINDOW2)" >&2
    exit 99
fi
if [ ! -x "$program" ]; then
    echo "error: $program is not an executable file" >&2
    exit 99
fi

tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/sw2-smoke.XXXXXX") || exit 99
trap 'rm -rf "$tmpdir"' EXIT
trap 'exit 99' HUP INT TERM

HOME="$tmpdir/home"
XDG_CONFIG_HOME="$HOME/.config"
XDG_RUNTIME_DIR="$tmpdir/runtime"
QT_QPA_PLATFORM=offscreen
export HOME XDG_CONFIG_HOME XDG_RUNTIME_DIR QT_QPA_PLATFORM
mkdir -p "$HOME" && mkdir -m 700 "$XDG_RUNTIME_DIR" || exit 99

"$program" --help > "$tmpdir/stdout" 2> "$tmpdir/stderr"
status=$?

result=0
# --help exits with status 1 after printing the usage.
# A larger status means a crash (e.g. 134 = SIGABRT, 139 = SIGSEGV).
if [ "$status" -gt 1 ]; then
    echo "FAIL: $program exited with status $status" >&2
    result=1
fi
if ! grep -q 'Usage:' "$tmpdir/stdout"; then
    echo "FAIL: 'Usage:' was not printed" >&2
    result=1
fi
for option in --transport --ws-path; do
    if ! grep -q -e "$option" "$tmpdir/stdout"; then
        echo "FAIL: '$option' was not printed" >&2
        result=1
    fi
done
if grep -q 'QObject::connect' "$tmpdir/stderr"; then
    echo "FAIL: QObject::connect warnings were printed" >&2
    result=1
fi

if [ "$result" -ne 0 ]; then
    echo "----- stdout -----" >&2
    cat "$tmpdir/stdout" >&2
    echo "----- stderr -----" >&2
    cat "$tmpdir/stderr" >&2
else
    echo "PASS: $program --help"
fi
exit "$result"
