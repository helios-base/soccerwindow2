#!/bin/sh
#
# check_desktop_entry.sh -- check the desktop entry (soccerwindow2.desktop.in)
#
# Usage: check_desktop_entry.sh [TOP_SRCDIR]
#
# TOP_SRCDIR is the top source directory (default: the parent directory of
# the directory of this script).
#
# The following points are checked:
#   - Exec of the application runs the executable in the directory given by
#     the placeholder @SOCCERWINDOW2_BINDIR@, which is replaced with
#     PREFIX/bin at the installation.
#   - the action "connect" runs "soccerwindow2 -c -d" (the same as sswindow2).
#   - Icon is the name of an application icon (icons/app/svg/NAME.svg), and
#     StartupWMClass is soccerwindow2.
#   - the desktop entry made from the template with a sample directory is
#     valid for desktop-file-validate (desktop-file-utils).
#     If desktop-file-validate is not found, the test is skipped after the
#     other checks.
#
# Exit status: 0 = pass, 1 = fail, 77 = skip, 99 = error (Automake convention).

LC_ALL=C
export LC_ALL

top="${1:-$(dirname "$0")/..}"
template="$top/soccerwindow2.desktop.in"
if [ ! -f "$template" ]; then
    echo "error: $template does not exist" >&2
    exit 99
fi

result=0

fail()
{
    echo "FAIL: $*" >&2
    result=1
}

# value KEY [GROUP]
#   print the value of KEY in GROUP (default: "Desktop Entry") of the template
value()
{
    awk -v key="$1" -v group="${2:-Desktop Entry}" '
        /^\[/ { current = substr( $0, 2, length( $0 ) - 2 ); next }
        current == group && index( $0, key "=" ) == 1 {
            print substr( $0, length( key ) + 2 )
            exit
        }' "$template"
}

expected='"@SOCCERWINDOW2_BINDIR@/soccerwindow2"'
if [ "$(value Exec)" != "$expected" ]; then
    fail "Exec must be $expected: $(value Exec)"
fi

case ";$(value Actions)" in
    *";connect;"*) ;;
    *) fail "Actions must contain connect: $(value Actions)" ;;
esac
expected='"@SOCCERWINDOW2_BINDIR@/soccerwindow2" -c -d'
if [ "$(value Exec "Desktop Action connect")" != "$expected" ]; then
    fail "Exec of the action connect must be $expected: $(value Exec "Desktop Action connect")"
fi
if [ -z "$(value Name "Desktop Action connect")" ]; then
    fail "the action connect has no Name"
fi

icon=$(value Icon)
if [ -z "$icon" ] || [ ! -f "$top/icons/app/svg/$icon.svg" ]; then
    fail "Icon must be the name of an application icon in icons/app/svg/: $icon"
fi
if [ "$(value StartupWMClass)" != soccerwindow2 ]; then
    fail "StartupWMClass must be soccerwindow2: $(value StartupWMClass)"
fi

if ! command -v desktop-file-validate >/dev/null 2>&1; then
    if [ "$result" -ne 0 ]; then
        exit 1
    fi
    echo "SKIP: desktop-file-validate (desktop-file-utils) is not found"
    exit 77
fi

tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/sw2-desktop.XXXXXX") || exit 99
trap 'rm -rf "$tmpdir"' EXIT
trap 'exit 99' HUP INT TERM

# the same replacement as the installation (a sample directory with a space)
sed -e 's|@SOCCERWINDOW2_BINDIR@|/home/sample user/.local/bin|g' "$template" > "$tmpdir/soccerwindow2.desktop"
if ! desktop-file-validate "$tmpdir/soccerwindow2.desktop"; then
    fail "desktop-file-validate reported errors"
fi

if [ "$result" -eq 0 ]; then
    echo "PASS: $template"
fi
exit "$result"
