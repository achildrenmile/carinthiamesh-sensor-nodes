#!/usr/bin/env python3
"""Erzeugt site/code/<beispiel>.js: der kompilierte Teil (app.h), die komplette main.cpp und die Pin-Flags je Board.
Aufruf: gen_code.py <beispiel> <app.h> <main.cpp> <pin_flags.sh> <ausgabe.js>"""
import json, subprocess, sys
ex, app_h, main_cpp, pin_sh, out = sys.argv[1:6]
boards = ["Heltec_v3_sensor", "RAK_4631_sensor", "Xiao_nrf52_sensor"]
flags = {b: subprocess.run([pin_sh, b, ex], capture_output=True, text=True, check=True).stdout.strip() for b in boards}
data = {"app": open(app_h).read(), "full": open(main_cpp).read(), "flags": flags}
open(out, "w").write("window.SRC=window.SRC||{};window.SRC[%s]=%s;\n" % (json.dumps(ex), json.dumps(data)))
