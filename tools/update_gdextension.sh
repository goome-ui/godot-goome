#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
ver="${1:-}"
if [ -z "$ver" ]; then
  echo "usage: tools/update_gdextension.sh <godot version, like 4.7.2-stable>"
  exit 1
fi
case "$(uname -s)-$(uname -m)" in
Linux-x86_64) pick=linux.x86_64 ;;
Linux-aarch64) pick=linux.arm64 ;;
*)
  echo "this only runs on linux, it needs the linux godot to dump the header"
  exit 1
  ;;
esac
work=$(mktemp -d "${TMPDIR:-$PWD}/gdext.XXXXXX")
trap 'rm -rf "$work"' EXIT
curl -fsSL -o "$work/godot.zip" "https://github.com/godotengine/godot/releases/download/$ver/Godot_v${ver}_$pick.zip"
curl -fsSL -o "$work/LICENSE.txt" "https://raw.githubusercontent.com/godotengine/godot/$ver/LICENSE.txt"
unzip -q "$work/godot.zip" -d "$work"
(cd "$work" && "./Godot_v${ver}_$pick" --headless --dump-gdextension-interface > /dev/null 2>&1)
test -s "$work/gdextension_interface.h" || { echo "godot didnt write gdextension_interface.h"; exit 1; }
cp "$work/gdextension_interface.h" native/gdextension/gdextension_interface.h
cp "$work/LICENSE.txt" native/gdextension/LICENSE.txt
build=$("$work/Godot_v${ver}_$pick" --headless --version 2>/dev/null | tail -1)
echo "$ver ${build##*.}" > native/gdextension/VERSION
echo "gdextension_interface.h from godot $(cat native/gdextension/VERSION)"
