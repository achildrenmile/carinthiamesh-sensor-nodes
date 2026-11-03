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

pins() {   # Pin-Flags je Target und Beispiel (Heltec V3 = Defaults im Code)
  case "$TARGET:$1" in
    RAK_4631_sensor:button-sensor) echo "-D BTN1_PIN=WB_IO1 -D BTN2_PIN=WB_IO2 -D BTN3_PIN=WB_IO3 -D BTN4_PIN=WB_IO4 -D LED_PIN=WB_IO5 -D BUZZER_PIN=WB_IO6";;
    RAK_4631_sensor:door-sensor)   echo "-D DOOR_PIN=WB_IO1";;
    RAK_4631_sensor:leak-sensor)   echo "-D LEAK_PIN=WB_IO1 -D LEAK_WET_LEVEL=HIGH";;
    Xiao_nrf52_sensor:button-sensor) echo "-D BTN1_PIN=D0 -D BTN2_PIN=D8 -D BTN3_PIN=D9 -D BTN4_PIN=D10 -D LED_PIN=D6 -D BUZZER_PIN=D7";;
    Xiao_nrf52_sensor:door-sensor) echo "-D DOOR_PIN=D0";;
    Xiao_nrf52_sensor:leak-sensor) echo "-D LEAK_PIN=D0";;
    *) echo "";;
  esac
}

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
