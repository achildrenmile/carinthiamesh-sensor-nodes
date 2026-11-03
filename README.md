# carinthiamesh-sensor-nodes

Beispiele für Sensor- und Aktor-Nodes im [MeshCore](https://github.com/meshcore-dev/MeshCore)-Netz, gesammelt von der CarinthiaMesh-Initiative in Kärnten. Jedes Beispiel ist ein kleiner, nachbaubarer Aufbau: Hardware, Schaltplan, Code und eine Schritt-für-Schritt-Anleitung.

*English: A collection of MeshCore sensor and actuator node examples from the CarinthiaMesh initiative (Carinthia, Austria). Each example ships wiring, a schematic, firmware changes and a step-by-step guide. Documentation is in German.*

## Beispiele

| Beispiel | Hardware | Was es macht | Status |
| --- | --- | --- | --- |
| [button-sensor](examples/button-sensor/) | Heltec V3, 4 Taster, LED, Buzzer | Tastendruck als DM-Alert, LED und Buzzer per Admin-Befehl schaltbar | kompiliert, nicht auf Hardware getestet |
| [door-sensor](examples/door-sensor/) | Heltec V3, Reed-Kontakt | Tür oder Fenster auf/zu als DM-Alert | kompiliert, nicht auf Hardware getestet |
| [leak-sensor](examples/leak-sensor/) | Heltec V3, Wassersensor-Modul | Wasser erkannt als High-Priority-Alert bis zum ACK | kompiliert, nicht auf Hardware getestet |
| [climate-sensor](examples/climate-sensor/) | Heltec V3, BME280 | Temperatur, Feuchte, Druck per Telemetrie, Alerts bei Frost und hoher Feuchte | kompiliert, nicht auf Hardware getestet |

Jedes Beispiel hat eine eigene Stückliste (`BOM.md`). Verwandte Projekte aus der Community stehen in [COMMUNITY.md](COMMUNITY.md).

Weitere Beispiele sind willkommen, siehe [Mitmachen](#mitmachen).

## Getting Started (Schritt für Schritt)

Für alle, die noch nie eine Firmware gebaut haben. Ihr braucht einen PC (Windows, macOS oder Linux), ein USB-Datenkabel und etwa eine Stunde, davon dauert der erste Build 5 bis 15 Minuten.

### 1. Hardware besorgen

Pro Beispiel gibt es eine Stückliste (`examples/<name>/BOM.md`). Zum Einstieg reichen ein **Heltec WiFi LoRa 32 V3 (868 MHz)** mit Antenne und die Bauteile des Beispiels. **Nie ohne Antenne senden**, das kann das Funkmodul beschädigen.

### 2. Software installieren

1. [Visual Studio Code](https://code.visualstudio.com/) installieren.
2. In VS Code links auf Erweiterungen klicken (Würfel-Symbol), **PlatformIO IDE** suchen und installieren. Danach VS Code neu starten. Beim ersten Start lädt PlatformIO einige Minuten lang Werkzeuge nach.
3. [Git](https://git-scm.com/downloads) installieren.

### 3. Code holen

In einem Terminal (unter Windows „Git Bash“ oder PowerShell) in einen Ordner eurer Wahl wechseln und ausführen:

```bash
git clone https://github.com/meshcore-dev/MeshCore.git
git clone https://github.com/achildrenmile/carinthiamesh-sensor-nodes.git
```

Ihr habt jetzt zwei Ordner nebeneinander: `MeshCore` (die Firmware) und `carinthiamesh-sensor-nodes` (diese Beispiele).

### 4. Beispiel einspielen

Das Beispiel ersetzt eine Datei der MeshCore-Vorlage. Hier am Beispiel `door-sensor`:

```bash
cd MeshCore
cp ../carinthiamesh-sensor-nodes/examples/door-sensor/firmware/main.cpp examples/simple_sensor/main.cpp
```

Unter Windows (PowerShell) statt `cp`: `copy ..\carinthiamesh-sensor-nodes\examples\door-sensor\firmware\main.cpp examples\simple_sensor\main.cpp`

### 5. Name und Passwort setzen

Öffnet in VS Code den Ordner `MeshCore` (Datei → Ordner öffnen) und dann `variants/heltec_v3/platformio.ini`. Sucht den Abschnitt `[env:Heltec_v3_sensor]` und ändert:

- `ADVERT_NAME`: der Name, unter dem euer Sensor im Mesh erscheint, zum Beispiel `"Keller Tuer"`
- `ADMIN_PASSWORD`: **euer eigenes Passwort**. Mit dem melden sich Companions am Sensor an. Das Standardpasswort `password` kennt jeder.

### 6. Board anschließen und flashen

1. Heltec per USB mit dem PC verbinden (Datenkabel, nicht nur Ladekabel).
2. In VS Code unten in der blauen Leiste auf das Ameisen-Symbol (PlatformIO) klicken, dann **Project Tasks → Heltec_v3_sensor → Upload**. Oder im Terminal im Ordner `MeshCore`: `pio run -e Heltec_v3_sensor -t upload`
3. Warten, bis „SUCCESS“ erscheint. Der erste Durchlauf lädt viel nach und dauert am längsten.

Findet PlatformIO das Board nicht: anderes USB-Kabel oder anderen Port probieren. Unter Linux fehlt manchmal die Berechtigung, dann den eigenen Benutzer zur Gruppe `dialout` hinzufügen. Hilft das nicht, am Heltec die PRG-Taste gedrückt halten, kurz RST drücken und erst dann PRG loslassen. Danach den Upload wiederholen.

### 7. Am Sensor anmelden

1. In der MeshCore-Companion-App (Android, iOS oder Web) euren Companion-Node verbinden.
2. Der Sensor sendet beim Start eine Advert. Wartet kurz, bis er in der Kontaktliste auftaucht.
3. Auf den Sensor tippen und mit dem Admin-Passwort aus Schritt 5 anmelden. Ab jetzt bekommt ihr dessen Alerts als Direktnachricht.

### 8. Testen

Jedes Beispiel hat in seiner `anleitung.md` einen Testabschnitt. Beim `door-sensor`: Magnet vom Kontakt wegnehmen, in der App erscheint „Tuer offen“. Kommt nichts an, den Sensor per USB anschließen und den seriellen Monitor in PlatformIO öffnen (Stecker-Symbol, 115200 Baud), dort stehen Fehlermeldungen.

### Anderes Board?

Die Beispiele sind für den Heltec V3 voreingestellt, laufen aber auch auf anderen Boards, wenn die Pins per Build-Flag angepasst werden. Geprüft sind die Builds für:

| Board | Target | Hinweis |
| --- | --- | --- |
| Heltec WiFi LoRa 32 V3 | `Heltec_v3_sensor` | Standard, Pins aus den READMEs |
| RAK4631 (WisBlock) | `RAK_4631_sensor` | Pins per `-D …=WB_IO1` usw. |
| Seeed XIAO nRF52840 + Wio-SX1262 | `Xiao_nrf52_sensor` | Target selbst anlegen: [`targets/xiao_nrf52_sensor.ini`](targets/xiao_nrf52_sensor.ini) |

Alle drei wurden **nur kompiliert**, nicht an echter Hardware getestet.

## Wie Sensor-Nodes bei MeshCore funktionieren

Alle Beispiele bauen auf der offiziellen Vorlage [`examples/simple_sensor`](https://github.com/meshcore-dev/MeshCore/tree/main/examples/simple_sensor) auf. Eine Einführung gibt der [MeshCore-Blog: Sensor Node Intro](https://blog.meshcore.io/2026/09/14/sensor-intro).

- **Eigener Build pro Anwendung.** Für Sensor-Nodes gibt es keine fertige Firmware im Flasher. Man passt `main.cpp` an und baut mit PlatformIO.
- **Alerts gehen als Direktnachricht** an alle Companions, die sich mit dem Admin-Passwort am Sensor angemeldet haben. Niedrige Priorität sendet einmal, hohe wiederholt bis zum ACK.
- **Telemetrie** (Akku, Umweltsensoren) können angemeldete Companions abfragen.
- **Eigene Befehle** in `handleCustomCommand()` machen aus dem Sensor einen Aktor, gesteuert über die Admin-Konsole der App.

### Warum nicht mit der Standard-Firmware?

Keine der fertigen MeshCore-Firmwares (Companion, Repeater, Room Server, Sensor) fragt externe GPIOs ab. Ein Taster oder Kontakt an einem freien Pin wird dort einfach ignoriert. Dafür gibt es den offenen Feature-Request [GPIO support in MeshCore (#3522)](https://github.com/meshcore-dev/MeshCore/issues/3522), GPIOs künftig per CLI zu lesen und zu schalten. Bis das umgesetzt ist, braucht es eigenen Code wie in diesen Beispielen.

## Spielregeln für das Mesh

Solange Sensor und Empfänger nebeneinander liegen, ist alles Zero-Hop und das Netz merkt nichts. Sobald ein Node über Repeater läuft, kostet jede Nachricht Airtime auf allen Repeatern im Pfad. Ohne bekannten Pfad geht sie sogar per Flood durchs ganze Netz.

- Direktnachricht statt Public-Kanal: keine Sensor- oder Testdaten in öffentliche Kanäle
- Rate-Limit ist Pflicht: Entprellen, Cooldown und ein Stundenbudget
- Nur Änderungen melden, Messwerte selten und gesammelt
- Wenige Empfänger, denn jeder bekommt jede Meldung einzeln
- Neue Nodes kurz in der lokalen Community ankündigen

## Mitmachen

Ein neues Beispiel bekommt einen eigenen Ordner unter `examples/<name>/` mit:

| Datei | Inhalt |
| --- | --- |
| `BOM.md` | Stückliste |
| `README.md` | Steckbrief: Hardware, Build-Target, was gemeldet wird, Airtime-Schutz, getesteter MeshCore-Stand, Status |
| `anleitung.md` | Verdrahtung, Aufbau, Build, Test |
| `schaltplan.svg` | Schaltplan |
| `firmware/` | geänderte `main.cpp` und ein Patch gegen `simple_sensor` |

Jedes Beispiel muss die Spielregeln oben einhalten. Pull Requests und Issues sind willkommen, ebenso Rückmeldungen, ob ein Beispiel auf echter Hardware läuft.

## Lizenz

MIT, siehe [LICENSE](LICENSE). Die Firmware-Dateien basieren auf MeshCore (MIT, Scott Powell / rippleradios.com).
