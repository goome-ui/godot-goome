#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
godot="${1:-${GODOT:-godot}}"
rm -rf tests/project/addons
mkdir -p tests/project/addons
cp -r addons/goome tests/project/addons/goome
rm -f tests/project/icons_c.txt
cc="${CC:-$(command -v cc || command -v gcc || true)}"
if [ -n "$cc" ]; then
  mkdir -p tests/build
  "$cc" -std=c99 -O1 -Inative -o tests/build/icons tests/icons.c -lm
  tests/build/icons > tests/project/icons_c.txt
else
  echo "no c compiler, the icon hashes get checked against the ones in test.gd"
fi
"$godot" --headless --path tests/project --import > /dev/null 2>&1 || true
out=$("$godot" --headless --path tests/project --quit-after 80000 2>&1) || { echo "$out"; exit 1; }
echo "$out"
echo "$out" | grep -q "goome test passed"
