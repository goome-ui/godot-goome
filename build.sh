#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
what="${1:-}"
os=$(uname -s)
arch=$(uname -m)
case "$arch" in
x86_64 | amd64) arch=x86_64 ;;
aarch64 | arm64) arch=arm64 ;;
esac
flags=(-O2 -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers ${WERROR:+-Werror} -Inative -Inative/gdextension -fvisibility=hidden)
srcs=(src/main.c src/gd.c src/ui.c src/view.c src/goome.c)
out=addons/goome/bin
mkdir -p "$out"
if [ "$what" = win ] || [[ "$os" == MINGW* || "$os" == MSYS* || "$os" == CYGWIN* ]]; then
  cc="${CC:-x86_64-w64-mingw32-gcc}"
  [ "$what" = win ] || cc="${CC:-gcc}"
  lib=goome.windows.x86_64.dll
  shared=(-shared -static-libgcc)
elif [ "$os" = Darwin ]; then
  cc="${CC:-cc}"
  lib=libgoome.macos.$arch.dylib
  if [ "$what" = universal ]; then
    lib=libgoome.macos.universal.dylib
    flags+=(-arch arm64 -arch x86_64)
  fi
  shared=(-dynamiclib -install_name "@rpath/$lib")
else
  cc="${CC:-cc}"
  lib=libgoome.linux.$arch.so
  shared=(-shared -fPIC)
fi
"$cc" "${flags[@]}" "${shared[@]}" -o "$out/$lib" "${srcs[@]}"
echo "built $out/$lib"
