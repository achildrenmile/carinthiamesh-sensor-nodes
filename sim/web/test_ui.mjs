// Bedient die gebaute Seite in einem echten Browser (Chrome DevTools Protocol): anmelden, Tuer oeffnen,
// pruefen, dass der Alert im Handy ankommt. Optional Screenshot.
// Aufruf: node test_ui.mjs <site-Ordner> [screenshot.png]      Browser: CHROME=google-chrome (Standard: chromium)
import { spawn } from 'node:child_process';
import fs from 'node:fs'; import os from 'node:os'; import path from 'node:path';
const dir = path.resolve(process.argv[2] || '../build-web/site'), shot = process.argv[3];
const url = `file://${dir}/index.html?ex=door-sensor&board=Heltec_v3_sensor`;
const chr = spawn(process.env.CHROME || 'chromium', ['--headless=new', '--no-sandbox', '--disable-gpu', '--allow-file-access-from-files',
  '--remote-debugging-port=9333', '--user-data-dir=' + fs.mkdtempSync(path.join(os.tmpdir(), 'sim-ui-')), 'about:blank'], { stdio: 'ignore' });
const sleep = ms => new Promise(r => setTimeout(r, ms));
let ok = false;
try {
  let page; for (let i = 0; i < 75 && !page; i++) { try { page = (await (await fetch('http://127.0.0.1:9333/json')).json()).find(t => t.type === 'page'); } catch {} if (!page) await sleep(200); }
  if (!page) throw new Error('Browser nicht erreichbar');
  const ws = new WebSocket(page.webSocketDebuggerUrl); await new Promise(r => ws.onopen = r);
  let id = 0; const pend = {}; ws.onmessage = m => { const d = JSON.parse(m.data); if (d.id && pend[d.id]) pend[d.id](d); };
  const send = (method, params = {}) => new Promise(r => { const i = ++id; pend[i] = r; ws.send(JSON.stringify({ id: i, method, params })); });
  const ev = async expr => (await send('Runtime.evaluate', { expression: expr, returnByValue: true })).result.result.value;
  await send('Page.enable'); await send('Emulation.setDeviceMetricsOverride', { width: 1200, height: 1250, deviceScaleFactor: 1, mobile: false });
  await send('Page.navigate', { url }); await sleep(3000);
  await ev(`const s=document.getElementById('speed'); s.value='60'; s.dispatchEvent(new Event('change'))`);
  await ev(`document.getElementById('login').click()`); await sleep(3000);
  const status = await ev(`document.getElementById('phonestat').textContent`);
  await ev(`document.getElementById('door').dispatchEvent(new MouseEvent('click',{bubbles:true}))`); await sleep(3000);
  await ev(`const s=document.getElementById('speed'); s.value='10'; s.dispatchEvent(new Event('change'))`);
  const msgs = await ev(`document.getElementById('msgs').innerText`);
  const rows = await ev(`document.querySelectorAll('#log tr').length`);
  console.log(`Status: ${status} | Meldungen: ${JSON.stringify(msgs)} | Pakete im Log: ${rows}`);
  ok = /angemeldet/.test(status) && !/nicht angemeldet/.test(status) && /Tuer offen/.test(msgs) && rows > 0;
  if (shot) { await ev('window.scrollTo(0,0)'); const png = await send('Page.captureScreenshot', { format: 'png' }); fs.writeFileSync(shot, Buffer.from(png.result.data, 'base64')); }
  console.log(ok ? 'OK: Seite bedienbar, Alert kommt im Handy an' : 'FEHLER: Alert nicht angekommen');
} catch (e) { console.log('FEHLER:', e.message); }
chr.kill(); process.exit(ok ? 0 : 1);
