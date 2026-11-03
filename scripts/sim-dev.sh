#!/bin/bash
# Beispiel aendern und im Simulator neu bauen.
# Aufruf:  scripts/sim-dev.sh [--setup | --once] [Beispiel ...]
#   ohne Option : baut, startet den Simulator auf http://localhost:8000 und baut bei jeder Aenderung
#                 an examples/*/firmware/main.cpp neu (Seite mit ?dev=1 laedt dann selbst nach)
#   --once      : einmal bauen und beenden (z. B. fuer die CI)
#   --setup     : nur MeshCore, Bibliotheken und Emscripten bereitstellen
# Umgebung: MESHCORE (Standard ~/MeshCore), PORT (8000), BUILD (build-dev)
set -e
HERE="$(cd "$(dirname "$0")/.." && pwd)"
MODE=watch; case "$1" in --setup) MODE=setup; shift;; --once) MODE=once; shift;; esac
EXAMPLES="${*:-door-sensor leak-sensor climate-sensor button-sensor}"
MESHCORE="${MESHCORE:-$HOME/MeshCore}"; PORT="${PORT:-8000}"; BUILD="${BUILD:-build-dev}"
REF="$(grep -m1 'MESHCORE_REF:' "$HERE/.github/workflows/ci.yml" | awk '{print $2}')"
EMVER="$(grep -m1 'EMSDK_VERSION:' "$HERE/.github/workflows/ci.yml" | awk '{print $2}')"

say() { printf '\n== %s\n' "$*"; }

if [ ! -d "$MESHCORE/.git" ]; then
  say "MeshCore ($REF) holen nach $MESHCORE"
  git clone -q https://github.com/meshcore-dev/MeshCore.git "$MESHCORE"
fi
git -C "$MESHCORE" checkout -q "$REF"

if [ ! -d "$MESHCORE/.pio/libdeps/Heltec_v3_sensor/Crypto" ]; then
  say "Bibliotheken (Crypto, CayenneLPP) laden"
  command -v pio >/dev/null || pip install --user -q platformio
  export PATH="$HOME/.local/bin:$PATH"
  (cd "$MESHCORE" && pio pkg install -e Heltec_v3_sensor >/dev/null)
fi

if ! command -v em++ >/dev/null; then
  EMSDK="${EMSDK:-$HOME/emsdk}"
  if [ ! -d "$EMSDK" ]; then
    say "Emscripten $EMVER installieren nach $EMSDK (einmalig, einige Minuten)"
    git clone -q --depth 1 https://github.com/emscripten-core/emsdk.git "$EMSDK"
  fi
  [ -x "$EMSDK/upstream/emscripten/em++" ] || { "$EMSDK/emsdk" install "$EMVER" >/dev/null; "$EMSDK/emsdk" activate "$EMVER" >/dev/null; }
  # shellcheck disable=SC1091
  source "$EMSDK/emsdk_env.sh" >/dev/null 2>&1
fi
[ "$MODE" = setup ] && { say "Fertig. Jetzt: scripts/sim-dev.sh door-sensor"; exit 0; }

build() {
  local out rc=0
  out="$(make --no-print-directory -C "$HERE/sim" -j"$(nproc)" CXX=em++ CC=emcc BUILD="$BUILD" MESHCORE="$MESHCORE" EXAMPLES="$EXAMPLES" web 2>&1)" || rc=$?
  printf '%s\n' "$out" | grep -vE '^(em\+\+|emcc|cp |python3 |printf |\./gen_app|mkdir )' || true
  return $rc
}
say "Bauen: $EXAMPLES"
ok=0; build && echo "OK" || { echo "FEHLER beim Bauen (siehe oben)"; ok=1; }
[ "$MODE" = once ] && exit $ok

python3 -m http.server "$PORT" -d "$HERE/sim/$BUILD/site" >/dev/null 2>&1 &
SRV=$!; trap 'kill $SRV 2>/dev/null' EXIT
say "Simulator: http://localhost:$PORT/?dev=1   (Strg+C beendet)"
say "Aendere jetzt examples/<name>/firmware/main.cpp, der Simulator wird neu gebaut."
sig() { cat "$HERE"/examples/*/firmware/main.cpp | cksum; }
last="$(sig)"
while sleep 1; do
  now="$(sig)"
  [ "$now" = "$last" ] && continue
  last="$now"; say "Aenderung erkannt, baue neu"
  build && echo "OK, Seite laedt nach" || echo "FEHLER beim Bauen, die Seite zeigt den letzten funktionierenden Stand"
done
