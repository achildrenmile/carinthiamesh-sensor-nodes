// Prueft die WebAssembly-Builds in Node: je Beispiel und Board einmal das Kernszenario mit den Pins des Boards.
// Aufruf: node test_web.cjs <Ordner mit wasm/>
const path = require('path'), fs = require('fs');
const dir = path.resolve(process.argv[2] || '../build-web/site');
const PINS = JSON.parse(fs.readFileSync(path.join(__dirname, 'pins.json')));
const EX = ['door-sensor', 'leak-sensor', 'climate-sensor', 'button-sensor'];

async function open(ex, bd) {
  const m = await require(path.join(dir, 'wasm', `${ex}-${bd}.js`))();
  const f = n => m.cwrap(n, null, ['number', 'number', 'number', 'number']);
  return {
    start: f('sim_start'), run: f('sim_run'), login: f('sim_login'), pinSet: f('sim_pin_set'), pinRel: f('sim_pin_release'),
    env: f('sim_env'), pinOut: m.cwrap('sim_pin_out', 'number', ['number']),
    events: () => JSON.parse(m.ccall('sim_events', 'string', [], [])),
  };
}
async function scenario(ex, bd) {
  const P = PINS[bd], s = await open(ex, bd);
  if (ex === 'door-sensor') s.pinSet(P.door, 0);
  if (ex === 'leak-sensor') s.pinSet(P.leak, 1 - P.wet);
  if (ex === 'climate-sensor') s.env(1, 20, 50, 1013);
  s.start(0, 0);
  for (let i = 0; i < 5 && !s.events().logged_in; i++) { s.login(); s.run(15000); }
  const msgs = []; let ledSeen = false;
  const take = () => msgs.push(...s.events().messages.map(m => m.text));
  take();
  if (ex === 'door-sensor') { s.pinRel(P.door); s.run(30000); }
  if (ex === 'leak-sensor') { s.pinSet(P.leak, P.wet); s.run(30000); }
  if (ex === 'climate-sensor') { s.env(1, 1, 50, 1013); s.run(60000); }
  if (ex === 'button-sensor') {
    s.pinSet(P.btn[1], 0); s.run(100); ledSeen = s.pinOut(P.led) === 1; s.run(30000);
  }
  take();
  const want = { 'door-sensor': 'Tuer offen', 'leak-sensor': 'Wasser erkannt!', 'climate-sensor': 'Frostgefahr', 'button-sensor': 'Taster 2 gedrueckt' }[ex];
  const ok = msgs.includes(want) && (ex !== 'button-sensor' || ledSeen);
  console.log(`${ok ? 'OK    ' : 'FEHLER'} ${ex} auf ${bd}: ${JSON.stringify(msgs)}${ex === 'button-sensor' ? ' LED=' + ledSeen : ''}`);
  return ok;
}
(async () => {
  let bad = 0;
  for (const bd of Object.keys(PINS)) for (const ex of EX) if (!(await scenario(ex, bd))) bad++;
  console.log(bad ? `${bad} Kombination(en) fehlgeschlagen` : '12 von 12 Kombinationen bestanden');
  process.exit(bad ? 1 : 0);
})();
