#!/bin/bash
# Baut alle Beispiele fuer ein MeshCore-Target (Kompiliertest, kein Flashen).
# Aufruf:  scripts/build_examples.sh <MeshCore-Ordner> <Target> [Beispiel ...]
# Targets: Heltec_v3_sensor | RAK_4631_sensor | Xiao_nrf52_sensor
set -e
MC="$(cd "$1" && pwd)"; TARGET="$2"; shift 2
HERE="$(cd "$(dirname "$0")/.." && pwd)"
EXAMPLES="${*:-button-sensor door-sensor leak-sensor climate-sensor}"

# Xiao: Target aus targets/ anlegen (Upstream hat keins)
if [ "$TARGET" = "Xiao_nrf52_sensor" ] && ! grep -q "env:Xiao_nrf52_sensor" "$MC/variants/xiao_nrf52/platformio.ini"; then
  printf '\n' >> "$MC/variants/xiao_nrf52/platformio.ini"
  cat "$HERE/targets/xiao_nrf52_sensor.ini" >> "$MC/variants/xiao_nrf52/platformio.ini"
fi

pins() { "$HERE/scripts/pin_flags.sh" "$TARGET" "$1"; }

cd "$MC"
trap 'git checkout -q examples/simple_sensor/main.cpp variants/xiao_nrf52/platformio.ini 2>/dev/null || true' EXIT
fail=0
for ex in $EXAMPLES; do
  cp "$HERE/examples/$ex/firmware/main.cpp" examples/simple_sensor/main.cpp
  if PLATFORMIO_BUILD_FLAGS="$(pins "$ex")" pio run -e "$TARGET" > "/tmp/build-$TARGET-$ex.log" 2>&1; then
    echo "OK    $TARGET $ex"
  else
    echo "FEHLER $TARGET $ex (siehe /tmp/build-$TARGET-$ex.log)"; tail -20 "/tmp/build-$TARGET-$ex.log"; fail=1
  fi
done
exit $fail
