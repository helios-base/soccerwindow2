#!/bin/sh
#
# Check whether soccerwindow2 is configured with the WebSocket connection
# (Qt WebSockets) as expected. Used by the CI.
#
# Usage: check_websocket.sh CONFIG_H EXPECTED
#
#   CONFIG_H  config.h made by CMake (e.g. build-qt6/config.h) or configure
#   EXPECTED  yes or no
#
# Exit status: 0 = as expected, 1 = not as expected, 99 = error.

config_h="$1"
expected="$2"

if [ ! -r "$config_h" ] || { [ "$expected" != yes ] && [ "$expected" != no ]; }; then
    echo "usage: $0 CONFIG_H yes|no" >&2
    exit 99
fi

if grep -q '^#define HAVE_QT_WEBSOCKETS' "$config_h"; then
    built=yes
else
    built=no
fi

echo "the WebSocket connection (HAVE_QT_WEBSOCKETS in $config_h): $built (expected: $expected)"
test "$built" = "$expected"
