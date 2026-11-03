#!/bin/bash
# Pin-Flags je Target und Beispiel (Heltec V3 = Defaults im Code). Aufruf: pin_flags.sh <Target> <Beispiel>
# Wird vom Build-Skript und von der Web-Simulation gemeinsam benutzt.
case "$1:$2" in
  RAK_4631_sensor:button-sensor) echo "-D BTN1_PIN=WB_IO1 -D BTN2_PIN=WB_IO2 -D BTN3_PIN=WB_IO3 -D BTN4_PIN=WB_IO4 -D LED_PIN=WB_IO5 -D BUZZER_PIN=WB_IO6";;
  RAK_4631_sensor:door-sensor)   echo "-D DOOR_PIN=WB_IO1";;
  RAK_4631_sensor:leak-sensor)   echo "-D LEAK_PIN=WB_IO1 -D LEAK_WET_LEVEL=HIGH";;
  Xiao_nrf52_sensor:button-sensor) echo "-D BTN1_PIN=D0 -D BTN2_PIN=D8 -D BTN3_PIN=D9 -D BTN4_PIN=D10 -D LED_PIN=D6 -D BUZZER_PIN=D7";;
  Xiao_nrf52_sensor:door-sensor) echo "-D DOOR_PIN=D0";;
  Xiao_nrf52_sensor:leak-sensor) echo "-D LEAK_PIN=D0";;
  *) echo "";;
esac
