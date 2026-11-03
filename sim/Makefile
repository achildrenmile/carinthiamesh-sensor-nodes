# Simulation der Beispiele auf dem PC. Aufruf:  make MESHCORE=/pfad/zu/MeshCore CRYPTO=... CAYENNE=... test
MESHCORE ?= ../../MeshCore
CRYPTO   ?= $(MESHCORE)/.pio/libdeps/Heltec_v3_sensor/Crypto
CAYENNE  ?= $(MESHCORE)/.pio/libdeps/Heltec_v3_sensor/CayenneLPP/src
EXAMPLES ?= button-sensor door-sensor leak-sensor climate-sensor
BUILD    ?= build

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O1 -g -w -fpermissive -include stdint.h -include vector -MMD -MP
INC := -Imock -Isrc -I$(MESHCORE)/src -I$(MESHCORE)/src/helpers -I$(MESHCORE)/examples/simple_sensor \
       -I$(CRYPTO) -I$(CAYENNE) -I$(MESHCORE)/lib/ed25519
DEFS := -DADVERT_NAME='"Sim Sensor"' -DADVERT_LAT=0.0 -DADVERT_LON=0.0 -DADMIN_PASSWORD='"password"' \
        -DLORA_FREQ=869.618 -DLORA_BW=62.5 -DLORA_SF=8 -DENV_INCLUDE_GPS=0 -DFIRMWARE_BUILD_DATE='"sim"' \
        -DFIRMWARE_VERSION='"sim"' -DMAX_CONTACTS=32 -DENABLE_ADVERT_ON_BOOT=1 -DRP2040_PLATFORM=1

CORE_SRC := $(MESHCORE)/src/Dispatcher.cpp $(MESHCORE)/src/Mesh.cpp $(MESHCORE)/src/Utils.cpp \
  $(MESHCORE)/src/Packet.cpp $(MESHCORE)/src/Identity.cpp \
  $(addprefix $(MESHCORE)/src/helpers/,StaticPoolPacketManager.cpp ClientACL.cpp IdentityStore.cpp CommonCLI.cpp \
    ConfigSerializer.cpp DynamicConfigSerializer.cpp RegionMap.cpp AdvertDataHelpers.cpp TxtDataHelpers.cpp CommonRadioPrefs.cpp TransportKeyStore.cpp) \
  $(MESHCORE)/examples/simple_sensor/SensorMesh.cpp $(MESHCORE)/examples/simple_sensor/TimeSeriesData.cpp
CRYPTO_SRC := $(filter-out %Esp32.cpp %ESP32.cpp %/RNG.cpp %NoiseSource.cpp %/TransistorNoiseSource.cpp %/RingOscillatorNoiseSource.cpp, $(wildcard $(CRYPTO)/*.cpp))
CAYENNE_SRC := $(wildcard $(CAYENNE)/*.cpp)
ED_SRC := $(wildcard $(MESHCORE)/lib/ed25519/*.c)

LIBOBJ := $(patsubst %,$(BUILD)/lib/%.o,$(notdir $(CORE_SRC) $(CRYPTO_SRC) $(CAYENNE_SRC) src/sim_globals.cpp src/sim_crypto_rng.cpp))
.PHONY: all test clean
-include $(wildcard $(BUILD)/lib/*.d)
all: $(addprefix $(BUILD)/,$(addsuffix /sim_test,$(EXAMPLES)))
test: all
	@fail=0; for e in $(EXAMPLES); do echo "== $$e"; $(BUILD)/$$e/sim_test || fail=1; done; exit $$fail
clean:
	rm -rf $(BUILD)

define OBJRULE
$$(BUILD)/lib/$(notdir $(1)).o: $(1)
	@mkdir -p $$(BUILD)/lib
	$$(CXX) $$(CXXFLAGS) $$(INC) $$(DEFS) -c $$< -o $$@
endef
$(foreach f,$(CORE_SRC) $(CRYPTO_SRC) $(CAYENNE_SRC) src/sim_globals.cpp src/sim_crypto_rng.cpp,$(eval $(call OBJRULE,$(f))))
$(BUILD)/lib/%.c.o: ;
ED_OBJ := $(patsubst %,$(BUILD)/lib/%.o,$(notdir $(ED_SRC)))
$(BUILD)/lib/%.c.o:
	@mkdir -p $(BUILD)/lib
	$(CC) -O1 -w -I$(MESHCORE)/lib/ed25519 -c $(filter %/$*.c,$(ED_SRC)) -o $@

TESTS := $(wildcard tests/scenario_*.cpp)
define EXRULE
$$(BUILD)/$(1)/app.h: gen_app.sh ../examples/$(1)/firmware/main.cpp
	./gen_app.sh $(1) $$(BUILD)/$(1)
$$(BUILD)/$(1)/sim_test: $$(BUILD)/$(1)/app.h $$(LIBOBJ) $$(ED_OBJ) $$(TESTS) $$(wildcard src/*.h) tests/harness.h
	$$(CXX) $$(CXXFLAGS) $$(INC) -I$$(BUILD)/$(1) $$(DEFS) -DEXAMPLE_$(subst -,_,$(1))=1 -Itests $$(TESTS) tests/main.cpp $$(LIBOBJ) $$(ED_OBJ) -o $$@
endef
$(foreach e,$(EXAMPLES),$(eval $(call EXRULE,$(e))))

# ---- Web-Version (WebAssembly): make CXX=em++ CC=emcc BUILD=build-web web  (emsdk muss aktiv sein)
WEB_BOARDS := Heltec_v3_sensor RAK_4631_sensor Xiao_nrf52_sensor
WEB_OUT    ?= $(BUILD)/site/wasm
WEB_FLAGS  := -std=c++17 -O2 -w -fpermissive -include stdint.h -include vector
WEB_LINK   := -sMODULARIZE=1 -sEXPORT_NAME=createSim -sSINGLE_FILE=1 -sALLOW_MEMORY_GROWTH=1 -sENVIRONMENT=web,node \
              -sEXPORTED_FUNCTIONS=_malloc,_free -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,UTF8ToString
.PHONY: web
web: $(foreach e,$(EXAMPLES),$(foreach b,$(WEB_BOARDS),$(WEB_OUT)/$(e)-$(b).js))
	cp web/pins.json $(BUILD)/site/
	@mkdir -p $(BUILD)/site/code
	@for e in $(EXAMPLES); do python3 web/gen_code.py $$e $(BUILD)/$$e/app.h ../examples/$$e/firmware/main.cpp ../scripts/pin_flags.sh $(BUILD)/site/code/$$e.js; done
	python3 -c "import json,sys;h=open('web/index.html').read();p=json.dumps(json.load(open('web/pins.json')));open('$(BUILD)/site/index.html','w').write(h.replace('<script id=\"pins\" type=\"application/json\">null</script>','<script id=\"pins\" type=\"application/json\">'+p+'</script>'))"
	printf '{"meshcore":"%s","emscripten":"%s","built":"%s"}\n' "$$(git -C $(MESHCORE) rev-parse --short HEAD)" "$$($(CXX) --version | head -1 | sed 's/.*) //;s/ (.*//')" "$$(date +%s)" > $(BUILD)/site/build-info.json
define WEBRULE
$$(WEB_OUT)/$(1)-$(2).js: $$(BUILD)/$(1)/app.h $$(LIBOBJ) $$(ED_OBJ) web/web_api.cpp web/boards.h $$(wildcard src/*.h)
	@mkdir -p $$(WEB_OUT)
	$$(CXX) $$(WEB_FLAGS) $$(INC) -I$$(BUILD)/$(1) $$(DEFS) $$(shell ../scripts/pin_flags.sh $(2) $(1)) -include web/boards.h \
	  web/web_api.cpp $$(LIBOBJ) $$(ED_OBJ) $$(WEB_LINK) -o $$@
endef
$(foreach e,$(EXAMPLES),$(foreach b,$(WEB_BOARDS),$(eval $(call WEBRULE,$(e),$(b)))))
