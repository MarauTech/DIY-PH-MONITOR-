const state = {
    theme: localStorage.getItem('theme') || 'dark',
    history: [],
    limits: { min: 6.0, max: 8.0 },
    chartData: null
};

// Elements
const el = {
    themeToggle: document.getElementById('theme-toggle'),
    iconLight: document.getElementById('theme-icon-light'),
    iconDark: document.getElementById('theme-icon-dark'),
    statusDot: document.getElementById('status-dot'),
    statusText: document.getElementById('status-text'),
    lastUpdate: document.getElementById('last-update'),
    fwVersion: document.getElementById('fw-version')
};

// Initialize Theme
function initTheme() {
    document.documentElement.setAttribute('data-theme', state.theme);
    updateThemeIcon();
}

function toggleTheme() {
    state.theme = state.theme === 'dark' ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', state.theme);
    localStorage.setItem('theme', state.theme);
    updateThemeIcon();
    drawChart();
}

function updateThemeIcon() {
    if (state.theme === 'dark') {
        el.iconLight.style.display = 'block';
        el.iconDark.style.display = 'none';
    } else {
        el.iconLight.style.display = 'none';
        el.iconDark.style.display = 'block';
    }
}

el.themeToggle.addEventListener('click', toggleTheme);
initTheme();

// Helpers
function formatTime(seconds) {
    const d = Math.floor(seconds / (3600*24));
    const h = Math.floor((seconds % (3600*24)) / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    const s = Math.floor(seconds % 60);
    
    if (d > 0) return `${d}d ${h}h ${m}m`;
    return `${h.toString().padStart(2, '0')}:${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}`;
}

function phColor(ph) {
    if (ph < state.limits.min) return '#f87171'; // Acid / Red
    if (ph > state.limits.max) return '#60a5fa'; // Base / Blue
    return '#4ade80'; // Neutral / Green
}

function setPhColor(element, ph) {
    element.style.color = phColor(ph);
}

// Chart
function drawChart() {
    if (!state.chartData || state.chartData.length === 0) return;
    
    const canvas = document.getElementById('phChart');
    if (!canvas) return;
    
    const ctx = canvas.getContext('2d');
    const dpr = window.devicePixelRatio || 1;
    const rect = canvas.parentNode.getBoundingClientRect();
    
    canvas.width = rect.width * dpr;
    canvas.height = (rect.height || 220) * dpr;
    
    ctx.scale(dpr, dpr);
    canvas.style.width = `${rect.width}px`;
    canvas.style.height = `${rect.height || 220}px`;
    
    const width = rect.width;
    const height = rect.height || 220;
    
    ctx.clearRect(0, 0, width, height);
    
    const root = getComputedStyle(document.documentElement);
    const textColor = root.getPropertyValue('--text-muted').trim() || '#718096';
    const gridColor = root.getPropertyValue('--border-card').trim() || '#2d3748';
    
    const padding = { top: 20, right: 20, bottom: 30, left: 40 };
    const chartW = width - padding.left - padding.right;
    const chartH = height - padding.top - padding.bottom;
    
    const phValues = state.chartData.map(d => (d.p !== undefined ? d.p : d.ph));
    const phMin = Math.max(0, Math.min(...phValues, state.limits.min) - 0.5);
    const phMax = Math.min(14, Math.max(...phValues, state.limits.max) + 0.5);
    
    const mapX = (i) => padding.left + (i / Math.max(1, state.chartData.length - 1)) * chartW;
    const mapY = (ph) => padding.top + chartH - ((ph - phMin) / Math.max(0.1, phMax - phMin)) * chartH;
    
    // Draw Grid
    ctx.font = '11px Inter, sans-serif';
    ctx.fillStyle = textColor;
    ctx.textAlign = 'right';
    ctx.textBaseline = 'middle';
    
    const gridLines = [0, 2, 4, 6, 7, 8, 10, 12, 14].filter(v => v >= phMin && v <= phMax);
    
    ctx.lineWidth = 1;
    gridLines.forEach(val => {
        const y = mapY(val);
        ctx.beginPath();
        ctx.strokeStyle = val === 7 ? '#4a5568' : gridColor;
        if(val === 7) ctx.setLineDash([4, 4]); else ctx.setLineDash([]);
        ctx.moveTo(padding.left, y);
        ctx.lineTo(width - padding.right, y);
        ctx.stroke();
        ctx.fillText(val.toFixed(1), padding.left - 8, y);
    });
    ctx.setLineDash([]);
    
    // Draw Alarm Limits
    if (state.limits.min >= phMin && state.limits.min <= phMax) {
        const y = mapY(state.limits.min);
        ctx.beginPath();
        ctx.strokeStyle = '#f87171';
        ctx.setLineDash([3, 3]);
        ctx.moveTo(padding.left, y);
        ctx.lineTo(width - padding.right, y);
        ctx.stroke();
    }
    if (state.limits.max >= phMin && state.limits.max <= phMax) {
        const y = mapY(state.limits.max);
        ctx.beginPath();
        ctx.strokeStyle = '#60a5fa';
        ctx.setLineDash([3, 3]);
        ctx.moveTo(padding.left, y);
        ctx.lineTo(width - padding.right, y);
        ctx.stroke();
    }
    ctx.setLineDash([]);

    // Draw Line and Area Gradient
    if (state.chartData.length > 0) {
        const grad = ctx.createLinearGradient(0, padding.top, 0, padding.top + chartH);
        grad.addColorStop(0, 'rgba(74, 222, 128, 0.25)');
        grad.addColorStop(1, 'rgba(74, 222, 128, 0.02)');
        
        ctx.beginPath();
        ctx.moveTo(mapX(0), mapY(phValues[0]));
        for(let i=1; i<state.chartData.length; i++) {
            ctx.lineTo(mapX(i), mapY(phValues[i]));
        }
        ctx.lineTo(mapX(state.chartData.length-1), padding.top + chartH);
        ctx.lineTo(mapX(0), padding.top + chartH);
        ctx.fillStyle = grad;
        ctx.fill();
        
        // Lines
        for(let i=1; i<state.chartData.length; i++) {
            const p1 = phValues[i-1];
            const p2 = phValues[i];
            
            ctx.beginPath();
            ctx.lineWidth = 2;
            ctx.strokeStyle = phColor((p1 + p2) / 2);
            ctx.moveTo(mapX(i-1), mapY(p1));
            ctx.lineTo(mapX(i), mapY(p2));
            ctx.stroke();
        }
        
        // Dot at current value
        const lastVal = phValues[phValues.length - 1];
        ctx.beginPath();
        ctx.fillStyle = phColor(lastVal);
        ctx.arc(mapX(state.chartData.length - 1), mapY(lastVal), 4, 0, Math.PI*2);
        ctx.fill();
    }
}

// Fetch Status
let lastAlarmState = null;

async function fetchStatus() {
    try {
        const res = await fetch('/api/status');
        if (res.status === 401) return;
        if (!res.ok) throw new Error('Status fetch failed');
        const data = await res.json();
        
        const now = new Date();
        el.lastUpdate.textContent = data.ntpSynced && data.ntpTime ? data.ntpTime : now.toLocaleTimeString();
        el.statusDot.className = 'dot connected pulse';
        el.statusText.textContent = data.wifiRSSI ? `WiFi (${data.wifiRSSI} dBm)` : 'Połączono';
        if(data.firmwareVersion) el.fwVersion.textContent = 'v' + data.firmwareVersion;
        if(data.ip) document.getElementById('sys-ip').textContent = data.ip;
        
        // pH Card
        const valPh = document.getElementById('val-ph');
        valPh.textContent = data.ph !== undefined ? data.ph.toFixed(2) : '--.--';
        setPhColor(valPh, data.ph || 7.0);
        
        if (data.minPH !== undefined && data.maxPH !== undefined) {
            document.getElementById('val-ph-min').textContent = data.minPH.toFixed(2);
            document.getElementById('val-ph-max').textContent = data.maxPH.toFixed(2);
        }
        
        // Voltage
        if (data.voltage !== undefined) {
            document.getElementById('val-voltage').textContent = data.voltage.toFixed(3) + ' V';
        }
        
        // Temp
        if (data.temperature !== null && data.temperature !== undefined && data.temperature > -50) {
            document.getElementById('val-temp').textContent = data.temperature.toFixed(1) + ' °C';
            if (data.minTemp !== undefined && data.minTemp > -50) {
                document.getElementById('val-temp-min').textContent = data.minTemp.toFixed(1);
                document.getElementById('val-temp-max').textContent = data.maxTemp.toFixed(1);
            }
        } else {
            document.getElementById('val-temp').textContent = 'Brak czujnika';
        }
        
        // Alarm
        const alarmBadge = document.getElementById('val-alarm');
        const isAlarm = (data.alarmState > 0);
        if (isAlarm) {
            alarmBadge.textContent = data.alarmState === 1 ? 'ALARM (NISKI)' : 'ALARM (WYSOKI)';
            alarmBadge.className = 'card-value status-badge badge-alarm';
            if(lastAlarmState === false) {
                if(Notification.permission === 'granted') {
                    new Notification('pH Monitor - ALARM!', { body: `pH przekroczyło limit: ${data.ph.toFixed(2)}` });
                }
            }
        } else {
            alarmBadge.textContent = 'OK (W NORMIE)';
            alarmBadge.className = 'card-value status-badge badge-ok';
        }
        lastAlarmState = isAlarm;
        
        // Uptime & RSSI
        document.getElementById('val-uptime').textContent = formatTime(data.uptime || 0);
        document.getElementById('val-rssi').textContent = data.wifiRSSI || '--';
        
        // Live calibration readout
        document.getElementById('cal-live-v').textContent = (data.voltage || 0).toFixed(3) + ' V';
        document.getElementById('cal-live-ph').textContent = (data.ph || 7.0).toFixed(2) + ' pH';
        
        // Live chart append if empty
        if (!state.chartData || state.chartData.length === 0) {
            state.chartData = [{ t: Math.floor(Date.now()/1000), p: data.ph }];
            drawChart();
        }
        
    } catch (err) {
        console.error(err);
        el.statusDot.className = 'dot disconnected';
        el.statusText.textContent = 'Brak połączenia';
    }
}

// Fetch Config
async function fetchConfig() {
    try {
        const res = await fetch('/api/config');
        if (!res.ok) return;
        const data = await res.json();
        
        document.getElementById('cfg-devname').value = data.deviceName || '';
        document.getElementById('cfg-ph-min').value = data.alarmLow !== undefined ? data.alarmLow : 6.0;
        document.getElementById('cfg-ph-max').value = data.alarmHigh !== undefined ? data.alarmHigh : 8.0;
        document.getElementById('cfg-hysteresis').value = data.hysteresis !== undefined ? data.hysteresis : 0.1;
        document.getElementById('cfg-alarm-hold').value = data.alarmHoldSec !== undefined ? data.alarmHoldSec : 15;
        
        document.getElementById('cfg-push-en').checked = !!data.pushoverEnabled;
        if (data.pushoverConfigured) {
            document.getElementById('cfg-push-user').placeholder = 'Skonfigurowano (wpisz nowy)';
            document.getElementById('cfg-push-token').placeholder = 'Skonfigurowano (wpisz nowy)';
        }
        
        document.getElementById('cfg-mqtt-en').checked = !!data.mqttEnabled;
        document.getElementById('cfg-mqtt-broker').value = data.mqttBroker || '';
        document.getElementById('cfg-mqtt-port').value = data.mqttPort || 1883;
        document.getElementById('cfg-mqtt-user').value = data.mqttUser || '';

        document.getElementById('admin-user').value = data.adminUser || 'admin';
        
        state.limits.min = data.alarmLow !== undefined ? data.alarmLow : 6.0;
        state.limits.max = data.alarmHigh !== undefined ? data.alarmHigh : 8.0;
        document.getElementById('val-alarm-lim').textContent = `${state.limits.min.toFixed(1)} / ${state.limits.max.toFixed(1)}`;
        
    } catch(err) {
        console.error('Błąd pobierania konfiguracji', err);
    }
}

// Fetch History
async function fetchHistory() {
    try {
        const res = await fetch('/api/history?limit=120');
        if(!res.ok) return;
        const data = await res.json();
        if (Array.isArray(data) && data.length > 0) {
            state.chartData = data;
            drawChart();
        }
    } catch(err) {
        console.error('Błąd pobierania historii', err);
    }
}

// Save config
document.getElementById('settings-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const btn = document.getElementById('btn-save-cfg');
    const status = document.getElementById('cfg-save-status');
    btn.disabled = true;
    
    const payload = {
        deviceName: document.getElementById('cfg-devname').value,
        alarmLow: parseFloat(document.getElementById('cfg-ph-min').value),
        alarmHigh: parseFloat(document.getElementById('cfg-ph-max').value),
        hysteresis: parseFloat(document.getElementById('cfg-hysteresis').value),
        alarmHoldSec: parseInt(document.getElementById('cfg-alarm-hold').value),
        pushoverEnabled: document.getElementById('cfg-push-en').checked,
        mqttEnabled: document.getElementById('cfg-mqtt-en').checked,
        mqttBroker: document.getElementById('cfg-mqtt-broker').value,
        mqttPort: parseInt(document.getElementById('cfg-mqtt-port').value),
        mqttUser: document.getElementById('cfg-mqtt-user').value
    };
    
    const pushUser = document.getElementById('cfg-push-user').value.trim();
    const pushToken = document.getElementById('cfg-push-token').value.trim();
    const mqttPass = document.getElementById('cfg-mqtt-pass').value.trim();
    
    if (pushUser.length > 0) payload.pushoverUser = pushUser;
    if (pushToken.length > 0) payload.pushoverToken = pushToken;
    if (mqttPass.length > 0) payload.mqttPass = mqttPass;

    try {
        const res = await fetch('/api/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        if(res.ok) {
            status.textContent = 'Zapisano pomyślnie!';
            status.style.color = '#4ade80';
            setTimeout(() => status.textContent = '', 3000);
            fetchConfig();
        } else {
            const errJson = await res.json().catch(() => ({}));
            status.textContent = errJson.error || 'Błąd zapisu';
            status.style.color = '#f87171';
        }
    } catch(err) {
        status.textContent = 'Błąd sieci';
        status.style.color = '#f87171';
    } finally {
        btn.disabled = false;
    }
});

// Admin form
document.getElementById('admin-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const btn = document.getElementById('btn-save-admin');
    const status = document.getElementById('admin-save-status');
    btn.disabled = true;
    
    const payload = {
        adminUser: document.getElementById('admin-user').value.trim()
    };
    const pass = document.getElementById('admin-pass').value.trim();
    if (pass.length > 0) payload.adminPass = pass;
    
    try {
        const res = await fetch('/api/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        if(res.ok) {
            status.textContent = 'Zapisano! Przeładuj stronę, aby zalogować się nowymi danymi.';
            status.style.color = '#4ade80';
        } else {
            const errJson = await res.json().catch(() => ({}));
            status.textContent = errJson.error || 'Błąd zapisu';
            status.style.color = '#f87171';
        }
    } catch(err) {
        status.textContent = 'Błąd połączenia';
        status.style.color = '#f87171';
    } finally {
        btn.disabled = false;
    }
});

// Test Pushover
document.getElementById('btn-test-push').addEventListener('click', async () => {
    const btn = document.getElementById('btn-test-push');
    btn.disabled = true;
    const oldText = btn.textContent;
    btn.textContent = 'Wysyłanie...';
    try {
        const res = await fetch('/api/pushover/test', { method: 'POST' });
        if (res.ok) {
            btn.textContent = 'Wysłano pomyślnie!';
        } else {
            const err = await res.json().catch(() => ({}));
            btn.textContent = err.message || 'Błąd wysyłania';
        }
        setTimeout(() => { btn.textContent = oldText; btn.disabled = false; }, 3000);
    } catch(err) {
        btn.textContent = 'Błąd połączenia';
        setTimeout(() => { btn.textContent = oldText; btn.disabled = false; }, 3000);
    }
});

// Reset Stats
document.getElementById('btn-reset-stats').addEventListener('click', async () => {
    if(confirm('Na pewno zresetować wartości Min/Max?')) {
        await fetch('/api/reset-stats', { method: 'POST' });
        fetchStatus();
    }
});

// Download CSV
document.getElementById('btn-dl-csv').addEventListener('click', () => {
    if(!state.chartData || state.chartData.length === 0) return;
    const csvRows = ['timestamp,ph,temperature,voltage,alarmState'];
    state.chartData.forEach(row => {
        const t = row.t || row.timestamp || '';
        const p = (row.p !== undefined ? row.p : row.ph) || 0;
        const te = (row.te !== undefined ? row.te : row.temperature) || '';
        const v = (row.v !== undefined ? row.v : row.voltage) || '';
        const a = (row.a !== undefined ? row.a : row.alarmState) || 0;
        csvRows.push(`${t},${p},${te},${v},${a}`);
    });
    const blob = new Blob([csvRows.join('\n')], { type: 'text/csv;charset=utf-8;' });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `ph_monitor_history_${new Date().toISOString().slice(0,10)}.csv`;
    a.click();
    window.URL.revokeObjectURL(url);
});

// Calibration
let calibPollInterval = null;

async function calibrate(type, customVal = 0) {
    const statusBox = document.getElementById('calib-status-box');
    statusBox.style.display = 'block';
    statusBox.textContent = 'Inicjalizacja kalibracji...';
    statusBox.style.backgroundColor = 'rgba(59, 130, 246, 0.15)';
    statusBox.style.color = '#60a5fa';
    
    let endpointType = type;
    if (type === '7.00') endpointType = 'neutral';
    else if (type === '4.01') endpointType = 'acid';
    else if (type === '9.18') endpointType = 'base';
    
    try {
        const res = await fetch('/api/calibrate', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ type: endpointType, customPH: customVal })
        });
        
        if(!res.ok) {
            throw new Error('Błąd rozpoczęcia kalibracji');
        }
        
        statusBox.textContent = 'Zbieranie i analiza stabilności próbek (5s)...';
        
        if(calibPollInterval) clearInterval(calibPollInterval);
        calibPollInterval = setInterval(pollCalibration, 500);
        
    } catch(err) {
        statusBox.textContent = 'Błąd: ' + err.message;
        statusBox.style.backgroundColor = 'rgba(239, 68, 68, 0.2)';
        statusBox.style.color = '#f87171';
    }
}

function calibrateCustom() {
    const val = parseFloat(document.getElementById('cal-custom-val').value);
    if(isNaN(val) || val < 0 || val > 14) {
        alert('Podaj prawidłową wartość pH bufora (0-14)');
        return;
    }
    calibrate('custom', val);
}

async function pollCalibration() {
    try {
        const res = await fetch('/api/calibrate/status');
        const data = await res.json();
        const statusBox = document.getElementById('calib-status-box');
        
        if (data.status === 'collecting') {
            statusBox.textContent = data.message || 'Trwa badanie stabilności sygnału...';
            statusBox.style.backgroundColor = 'rgba(59, 130, 246, 0.15)';
            statusBox.style.color = '#60a5fa';
        } else if (data.status === 'done') {
            clearInterval(calibPollInterval);
            statusBox.textContent = `${data.message || 'Kalibracja zakończona pomyślnie!'} (Napięcie: ${(data.voltage/1000).toFixed(3)} V, stdDev: ${(data.stdDev||0).toFixed(2)} mV)`;
            statusBox.style.backgroundColor = 'rgba(74, 222, 128, 0.2)';
            statusBox.style.color = '#4ade80';
            fetchStatus();
        } else if (data.status === 'failed') {
            clearInterval(calibPollInterval);
            statusBox.textContent = data.message || 'Błąd: Pomiar niestabilny. Odczekaj na ustabilizowanie sondy.';
            statusBox.style.backgroundColor = 'rgba(239, 68, 68, 0.2)';
            statusBox.style.color = '#f87171';
        }
    } catch(err) {
        console.error(err);
    }
}

// Request Notification Permission
if ("Notification" in window && Notification.permission !== "granted" && Notification.permission !== "denied") {
    Notification.requestPermission();
}

// Initialization and Loops
window.addEventListener('resize', drawChart);
fetchConfig().then(() => {
    fetchHistory();
    fetchStatus();
    setInterval(fetchStatus, 2000);
    setInterval(fetchHistory, 60000);
});
