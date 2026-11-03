#!/bin/bash
# Schneidet den Anwendungsteil (bis vor "StdRNG fast_rng;") aus examples/<name>/firmware/main.cpp
# und legt ihn als app.h ab. Der Rest von main.cpp (setup/loop) ist Board-Boilerplate.
set -e
ex="$1"; out="$2"
src="$(dirname "$0")/../examples/$ex/firmware/main.cpp"
mkdir -p "$out"
n=$(grep -n '^StdRNG fast_rng;' "$src" | cut -d: -f1)
head -n $((n-1)) "$src" > "$out/app.h"
