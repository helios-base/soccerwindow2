#!/bin/sh
#
# Smoke test: the export from the command line, without a display
# (docs/plan/2026-11-28-00_frame-export.md 4.5, S14).
#
# Usage: smoke_export.sh [/path/to/soccerwindow2]
#
# The program path is given by the first argument or by the environment
# variable SOCCERWINDOW2. The game log test/data/head_v5.rcg (3 frames) is
# found next to this script.
#
# The test passes when:
#   - --export-images writes the 3 images and exits with status 0,
#   - --auto-image-save writes the 3 images of the whole game log into
#     --image-save-dir and exits with status 0,
#   - an option of the export without --export-video or --export-images
#     exits with status 1 and tells why on the standard error, and
#   - ~/.soccerwindow2 is not written.
#
# soccerwindow2 itself chooses the offscreen platform for the export: the
# test runs it without DISPLAY, WAYLAND_DISPLAY and QT_QPA_PLATFORM.
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
game_log="$(cd "$(dirname "$0")" && pwd)/data/head_v5.rcg"
if [ ! -f "$game_log" ]; then
    echo "error: $game_log is not found" >&2
    exit 99
fi

tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/sw2-export.XXXXXX") || exit 99
trap 'rm -rf "$tmpdir"' EXIT
trap 'exit 99' HUP INT TERM

HOME="$tmpdir/home"
XDG_CONFIG_HOME="$HOME/.config"
XDG_RUNTIME_DIR="$tmpdir/runtime"
export HOME XDG_CONFIG_HOME XDG_RUNTIME_DIR
unset DISPLAY WAYLAND_DISPLAY QT_QPA_PLATFORM
mkdir -p "$HOME" && mkdir -m 700 "$XDG_RUNTIME_DIR" || exit 99

result=0

# run NAME EXPECTED_STATUS ARGS...: the output into $tmpdir/NAME.out and .err
run()
{
    name="$1"
    expected="$2"
    shift 2
    "$program" "$@" > "$tmpdir/$name.out" 2> "$tmpdir/$name.err"
    status=$?
    if [ "$status" -ne "$expected" ]; then
        echo "FAIL: $name: exited with status $status, not $expected" >&2
        echo "----- stderr of $name -----" >&2
        cat "$tmpdir/$name.err" >&2
        result=1
    fi
}

# expect_files NAME DIR FILES...: DIR has just these files
expect_files()
{
    name="$1"
    dir="$2"
    shift 2
    listed=$(cd "$dir" 2> /dev/null && ls | tr '\n' ' ')
    if [ "$listed" != "$* " ]; then
        echo "FAIL: $name: the files in $dir are [$listed], not [$* ]" >&2
        result=1
    fi
}

run images 0 --export-images "$tmpdir/images" --export-size 320x240 "$game_log"
expect_files images "$tmpdir/images" image-00000.png image-00001.png image-00002.png

run auto 0 --auto-image-save --image-save-dir "$tmpdir/auto" --image-name-prefix shot- \
    --canvas-size 320x240 "$game_log"
expect_files auto "$tmpdir/auto" shot-00000.png shot-00001.png shot-00002.png

run wrong 1 --export-fps 30 "$game_log"
if ! grep -q 'soccerwindow2: --export-fps is an option of the export' "$tmpdir/wrong.err"; then
    echo "FAIL: wrong: the reason was not told" >&2
    cat "$tmpdir/wrong.err" >&2
    result=1
fi

if [ -e "$HOME/.soccerwindow2" ]; then
    echo "FAIL: ~/.soccerwindow2 was written" >&2
    result=1
fi

if [ "$result" -eq 0 ]; then
    echo "PASS: $program --export-images, --auto-image-save"
fi
exit "$result"
