#ifndef WEB_INDEX_H
#define WEB_INDEX_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>EspCamLib Console</title>
    <style>
        :root {
            --bg: #000000;
            --panel: #1a1a1a;
            --border: #333;
            --text-main: #e0e0e0;
            --text-dim: #888;
            --accent: #0f0;
            --alert: #f00;
            --warn: #fa0;
        }

        body {
            background: var(--bg);
            color: var(--text-main);
            font-family: Consolas, Monaco, monospace;
            font-size: 13px;
            margin: 0;
            height: 100vh;
            display: grid;
            grid-template-rows: 40px 1fr 150px;
            overflow: hidden;
        }

        header {
            background: var(--panel);
            border-bottom: 1px solid var(--border);
            display: flex;
            align-items: center;
            padding: 0 15px;
            justify-content: space-between;
        }

        .brand { font-size: 16px; font-weight: 600; color: #fff; letter-spacing: 1px; }
        
        .status-badge { 
            background: #333; color: #aaa; padding: 4px 8px; 
            border-radius: 2px; font-size: 11px; font-weight: bold; 
        }
        .status-badge.online { background: var(--accent); color: #000; }

        .workspace {
            display: grid;
            grid-template-columns: 1fr 320px;
            overflow: hidden;
        }

        .viewport {
            background: #050505;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
            overflow: hidden;
        }

        .stream-box {
            width: 100%;
            max-width: 1280px;
            aspect-ratio: 16 / 9;
            background: #080808;
            border: 1px solid var(--border);
            position: relative;
            display: flex;
            justify-content: center;
            align-items: center;
            box-shadow: 0 0 20px rgba(0,0,0,0.5);
        }
        
        #cam-stream {
            width: 100%;
            height: 100%;
            object-fit: contain;
            display: block;
        }

        .overlay-data {
            position: absolute;
            top: 15px;
            left: 15px;
            display: flex;
            flex-direction: column;
            gap: 5px;
            pointer-events: none;
        }
        
        .overlay-tag {
            background: rgba(0, 0, 0, 0.7);
            padding: 4px 8px;
            border-left: 2px solid var(--accent);
            font-size: 11px;
            backdrop-filter: blur(4px);
        }

        aside {
            background: var(--bg);
            border-left: 1px solid var(--border);
            padding: 15px;
            display: flex;
            flex-direction: column;
            gap: 20px;
            overflow-y: auto;
        }

        .panel-box { border: 1px solid var(--border); padding: 10px; }
        .panel-header { 
            color: var(--text-dim); text-transform: uppercase; font-size: 11px; 
            margin-bottom: 10px; border-bottom: 1px solid #222; padding-bottom: 5px; 
            display: flex; justify-content: space-between;
        }

        .data-row { display: flex; justify-content: space-between; margin-bottom: 6px; }
        .data-val { color: var(--accent); font-weight: bold; }

        .control-group { margin-bottom: 15px; }
        .control-label { display: block; margin-bottom: 8px; color: var(--text-dim); }
        
        input[type="range"] {
            width: 100%;
            height: 4px;
            background: #333;
            outline: none;
            -webkit-appearance: none;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 14px; height: 14px; background: var(--text-main);
            border-radius: 50%; cursor: pointer; border: 2px solid #000;
        }

        button {
            background: #222; border: 1px solid #444; color: #fff;
            width: 100%; padding: 10px; cursor: pointer;
            font-family: inherit; margin-bottom: 5px;
            transition: all 0.2s;
        }
        button:hover { background: #333; border-color: #666; }
        button:active { background: var(--accent); color: #000; }
        button.danger { border-color: #500; color: #f55; }
        button.danger:hover { background: #500; color: #fff; }

        #log-panel {
            background: #000;
            border-top: 1px solid var(--border);
            padding: 10px;
            font-family: 'Courier New', Courier, monospace;
            overflow-y: auto;
            display: flex;
            flex-direction: column-reverse;
        }
        .log-line { margin: 2px 0; border-bottom: 1px solid #111; padding-bottom: 2px;}
        .log-time { color: var(--text-dim); margin-right: 10px; }
        .log-msg.err { color: var(--alert); }
        .log-msg.ok { color: var(--accent); }
    </style>
</head>
<body>

<header>
    <div class="brand">EspCamLib</div>
    <div id="status-badge" class="status-badge">DISCONNECTED</div>
</header>

<div class="workspace">
    <div class="viewport">
        <div class="stream-box">
            <img id="cam-stream" src="">
            <div class="overlay-data">
                <div class="overlay-tag" id="osd-res">RES: --</div>
                <div class="overlay-tag" id="osd-fps">FPS: --</div>
            </div>
        </div>
    </div>

    <aside>
        <div class="panel-box">
            <div class="panel-header">Performance Metrics</div>
            <div class="data-row"><span>RAM Usage</span> <span class="data-val" id="val-ram">--</span></div>
            <div class="data-row"><span>Latency</span> <span class="data-val" id="val-ping">--</span></div>
            <div class="data-row"><span>Bitrate</span> <span class="data-val" id="val-bitrate">--</span></div>
            <div class="data-row"><span>WiFi Signal</span> <span class="data-val" id="val-rssi">--</span></div>
        </div>

        <div class="panel-box">
            <div class="panel-header">Camera Controls</div>
            
            <div class="control-group">
                <span class="control-label">Flash Intensity (0-255)</span>
                <input type="range" min="0" max="255" value="0" id="flash-slider" oninput="updateFlashDebounced(this.value)">
            </div>

            <button onclick="sendCommand('hmirror')">Flip X</button>
            <button onclick="sendCommand('vflip')">Flip Y</button>
            <button class="danger" onclick="sendCommand('reboot')">Reboot</button>
        </div>
    </aside>
</div>

<div id="log-panel">
    <div class="log-line"><span class="log-time">System</span> <span class="log-msg">Interface ready.</span></div>
</div>

<script>
    const CONFIG = {
        streamPort: 81,
        apiPort: 80,
        interval: 1000
    };

    const ui = {
        badge: document.getElementById('status-badge'),
        stream: document.getElementById('cam-stream'),
        ram: document.getElementById('val-ram'),
        ping: document.getElementById('val-ping'),
        rssi: document.getElementById('val-rssi'),
        bitrate: document.getElementById('val-bitrate'),
        osdRes: document.getElementById('osd-res'),
        osdFps: document.getElementById('osd-fps'),
        logs: document.getElementById('log-panel')
    };

    let baseUrl = window.location.hostname;
    if (!baseUrl) baseUrl = "192.168.1.100"; 
    
    ui.stream.src = `http://${baseUrl}:${CONFIG.streamPort}/stream`;

    let frameCount = 0;
    let lastFrameTime = Date.now();
    let currentFps = 0;
    let flashTimeout = null;

    ui.stream.onload = () => {
        frameCount++;
        const now = Date.now();
        if (now - lastFrameTime >= 1000) {
            currentFps = frameCount;
            frameCount = 0;
            lastFrameTime = now;
            updateStreamStats();
        }
    };

    function updateStreamStats() {
        ui.osdFps.innerText = `FPS: ${currentFps}`;
        if (ui.stream.naturalWidth) {
            ui.osdRes.innerText = `RES: ${ui.stream.naturalWidth}x${ui.stream.naturalHeight}`;
        }
    }

    function addLog(msg, type = 'info') {
        const time = new Date().toLocaleTimeString();
        const line = document.createElement('div');
        line.className = 'log-line';
        line.innerHTML = `<span class="log-time">${time}</span> <span class="log-msg ${type}">${msg}</span>`;
        ui.logs.prepend(line);
        if (ui.logs.children.length > 50) ui.logs.lastElementChild.remove();
    }

    function updateFlashDebounced(val) {
        if (flashTimeout) clearTimeout(flashTimeout);
        flashTimeout = setTimeout(() => {
            updateFlash(val);
        }, 150);
    }

    async function updateFlash(val) {
        try {
            val = parseInt(val);
            await fetch(`http://${baseUrl}:${CONFIG.apiPort}/control?var=flash&val=${val}`);
        } catch (e) {
            console.error(e);
        }
    }

    async function sendCommand(cmd) {
        addLog(`Sending command: ${cmd}...`);
        try {
            await fetch(`http://${baseUrl}:${CONFIG.apiPort}/control?var=${cmd}&val=1`);
            addLog(`Command ${cmd} executed`, 'ok');
        } catch (e) {
            addLog(`Command ${cmd} failed`, 'err');
        }
    }

    async function fetchTelemetry() {
        const start = performance.now();
        try {
            const controller = new AbortController();
            setTimeout(() => controller.abort(), 2000);

            const res = await fetch(`http://${baseUrl}:${CONFIG.apiPort}/status`, { signal: controller.signal });
            const latency = Math.round(performance.now() - start);
            
            if (res.ok) {
                const data = await res.json();
                
                ui.badge.innerText = "ONLINE";
                ui.badge.className = "status-badge online";
                
                ui.ping.innerText = latency + " ms";
                ui.ram.innerText = data.heap ? Math.round(data.heap / 1024) + " KB" : "N/A";
                ui.rssi.innerText = data.rssi ? data.rssi + " dBm" : "N/A";
                
                if (data.kbps) ui.bitrate.innerText = data.kbps + " kbps";
                else ui.bitrate.innerText = "N/A"; 

            } else {
                throw new Error("API Error");
            }
        } catch (e) {
            ui.badge.innerText = "OFFLINE";
            ui.badge.className = "status-badge";
            ui.ping.innerText = "--";
        }
    }

    setInterval(fetchTelemetry, CONFIG.interval);
    addLog("Dashboard initialized");
</script>
</body>
</html>
)rawliteral";

const size_t INDEX_HTML_LENGTH = sizeof(INDEX_HTML) - 1;

#endif