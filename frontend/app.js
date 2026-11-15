
'use strict';

const POLL_MS = 1200;
const MAX_CARS_DRAWN = 7;
const CAR_GAP = 40;
const SVG_NS = 'http://www.w3.org/2000/svg';

const state = {
  roads: [],
  priority: null,
  simulation: 'STOPPED',
  cycle: 0,
  connected: null,
  cycleIntervalMs: 1200
};

let pollTimer = null;

const $ = (id) => document.getElementById(id);

const APPROACHES = [
  {
    key: 'N',
    carX: 286,
    carY: 228,
    dx: 0,
    dy: -1,
    vertical: true,
    lampX: 238,
    lampY: 222,
    tagX: 330,
    tagY: 32,
    waitX: 330,
    waitY: 46
  },
  {
    key: 'E',
    carX: 412,
    carY: 286,
    dx: 1,
    dy: 0,
    vertical: false,
    lampX: 418,
    lampY: 238,
    tagX: 612,
    tagY: 348,
    waitX: 612,
    waitY: 362,
    anchorEnd: true
  },
  {
    key: 'S',
    carX: 354,
    carY: 412,
    dx: 0,
    dy: 1,
    vertical: true,
    lampX: 402,
    lampY: 418,
    tagX: 296,
    tagY: 612,
    waitX: 296,
    waitY: 598
  },
  {
    key: 'W',
    carX: 228,
    carY: 354,
    dx: -1,
    dy: 0,
    vertical: false,
    lampX: 222,
    lampY: 402,
    tagX: 28,
    tagY: 300,
    waitX: 28,
    waitY: 314
  }
];

function svgEl(name, attrs) {
  const node = document.createElementNS(SVG_NS, name);

  for (const key in attrs) {
    node.setAttribute(key, attrs[key]);
  }

  return node;
}

function drawScenery() {
  const scenery = $('scenery');

  const corners = [
    { x: 24, y: 24 },
    { x: 388, y: 24 },
    { x: 24, y: 388 },
    { x: 388, y: 388 }
  ];

  corners.forEach((c, index) => {
    const bx = c.x + 46;
    const by = c.y + 46;

    scenery.appendChild(svgEl('rect', {
      x: bx,
      y: by,
      width: 136,
      height: 110,
      rx: 3,
      class: 'bldg'
    }));

    for (let row = 0; row < 3; row++) {
      for (let col = 0; col < 6; col++) {
        if ((row * 7 + col * 3 + index * 5) % 4 === 0) {
          continue;
        }

        scenery.appendChild(svgEl('rect', {
          x: bx + 13 + col * 20,
          y: by + 18 + row * 30,
          width: 10,
          height: 12,
          class: 'win'
        }));
      }
    }

    scenery.appendChild(svgEl('circle', {
      cx: c.x + 26,
      cy: c.y + 198,
      r: 9,
      class: 'tree'
    }));

    scenery.appendChild(svgEl('circle', {
      cx: c.x + 40,
      cy: c.y + 206,
      r: 7,
      class: 'tree'
    }));
  });

  const crossings = $('crossings');

  for (let i = 0; i < 8; i++) {
    crossings.appendChild(svgEl('rect', {
      x: 256 + i * 17,
      y: 222,
      width: 10,
      height: 22
    }));

    crossings.appendChild(svgEl('rect', {
      x: 256 + i * 17,
      y: 396,
      width: 10,
      height: 22
    }));

    crossings.appendChild(svgEl('rect', {
      x: 396,
      y: 256 + i * 17,
      width: 22,
      height: 10
    }));

    crossings.appendChild(svgEl('rect', {
      x: 222,
      y: 256 + i * 17,
      width: 22,
      height: 10
    }));
  }
}

const CAR_SLOT_GAP = 40;
const QUEUE_EASE = 0.20;
const RELEASE_SPEED = 130;
const EXIT_DISTANCE = 430;

const reduceMotion =
  window.matchMedia &&
  window.matchMedia('(prefers-reduced-motion: reduce)').matches;

APPROACHES.forEach(ap => {
  ap.queue = [];
  ap.moving = [];
  ap.pending = 0;
  ap.lastCount = null;
});

function makeCar(ap, emergency) {
  const el = svgEl('rect', {
    width: ap.vertical ? 16 : 26,
    height: ap.vertical ? 26 : 16,
    rx: 3,
    class: emergency ? 'car car-emg' : 'car'
  });

  $('cars').appendChild(el);

  return {
    el,
    pos: (MAX_CARS_DRAWN + 1) * CAR_SLOT_GAP,
    emergency
  };
}

function placeCar(ap, car) {
  const x = ap.carX + ap.dx * car.pos;
  const y = ap.carY + ap.dy * car.pos;

  car.el.setAttribute('x', ap.vertical ? x - 8 : x - 13);
  car.el.setAttribute('y', ap.vertical ? y - 13 : y - 8);
}

function syncCars() {
  APPROACHES.forEach((ap, index) => {
    const road = state.roads[index];
    const green = Boolean(road) && road.signalStatus === 'GREEN';

    if (!road) {
      ap.queue.concat(ap.moving).forEach(c => c.el.remove());
      ap.queue = [];
      ap.moving = [];
      ap.pending = 0;
      ap.lastCount = null;
      return;
    }

    if (ap.lastCount !== null && road.vehicleCount < ap.lastCount) {
      ap.pending += ap.lastCount - road.vehicleCount;
    }

    ap.lastCount = road.vehicleCount;

    let wanted = Math.min(road.vehicleCount, MAX_CARS_DRAWN) -
                 ap.moving.length;

    if (wanted < 0) {
      wanted = 0;
    }

    while (ap.queue.length < wanted) {
      ap.queue.push(makeCar(ap, false));
    }

    while (ap.queue.length > wanted) {
      if (green) {
        ap.moving.push(ap.queue.shift());
      } else {
        ap.queue.pop().el.remove();
      }
    }

    const alreadyHasOne =
      ap.moving.some(c => c.emergency) ||
      ap.queue.some(c => c.emergency);

    if (!road.emergencyStatus) {
      ap.queue.forEach(car => {
        if (car.emergency) {
          car.emergency = false;
          car.el.setAttribute('class', 'car');
        }
      });
    } else if (!alreadyHasOne && ap.queue.length > 0) {
      ap.queue[0].emergency = true;
      ap.queue[0].el.setAttribute('class', 'car car-emg');
    }
  });
}

let lastFrame = 0;

function animate(now) {
  const dt = lastFrame
    ? Math.min((now - lastFrame) / 1000, 0.1)
    : 0;

  lastFrame = now;

  APPROACHES.forEach((ap, index) => {
    const road = state.roads[index];
    const green = Boolean(road) && road.signalStatus === 'GREEN';

    ap.queue.forEach((car, i) => {
      const slot = i * CAR_SLOT_GAP;
      car.pos += (slot - car.pos) * QUEUE_EASE;
      placeCar(ap, car);
    });

    if (ap.pending > 0 && ap.queue.length > 0 && ap.queue[0].pos < 4) {
      ap.moving.push(ap.queue.shift());
      ap.pending--;
    }

    if (!green && ap.pending > 0) {
      ap.pending = 0;
    }

    const stillMoving = [];

    ap.moving.forEach(car => {
      car.pos -= RELEASE_SPEED * dt;

      if (car.pos < -EXIT_DISTANCE) {
        car.el.remove();
      } else {
        placeCar(ap, car);
        stillMoving.push(car);
      }
    });

    ap.moving = stillMoving;
  });

  requestAnimationFrame(animate);
}

function renderJunction() {
  const lampLayer = $('lights');
  const labelLayer = $('roadLabels');

  lampLayer.textContent = '';
  labelLayer.textContent = '';

  APPROACHES.forEach((ap, index) => {
    const road = state.roads[index];
    const green = Boolean(road) && road.signalStatus === 'GREEN';

    lampLayer.appendChild(svgEl('rect', {
      x: ap.lampX - 7,
      y: ap.lampY - 16,
      width: 14,
      height: 32,
      rx: 3,
      class: 'lamp-housing'
    }));

    lampLayer.appendChild(svgEl('circle', {
      cx: ap.lampX,
      cy: ap.lampY - 7,
      r: 4,
      class: green ? 'lamp-off' : 'lamp-red'
    }));

    lampLayer.appendChild(svgEl('circle', {
      cx: ap.lampX,
      cy: ap.lampY + 7,
      r: 4,
      class: green ? 'lamp-green' : 'lamp-off'
    }));

    if (!road) {
      return;
    }

    const tag = svgEl('text', {
      x: ap.tagX,
      y: ap.tagY,
      class: 'road-tag'
    });

    if (ap.anchorEnd) {
      tag.setAttribute('text-anchor', 'end');
    }

    tag.textContent = road.roadID;
    labelLayer.appendChild(tag);

    const wait = svgEl('text', {
      x: ap.waitX,
      y: ap.waitY,
      class: 'road-wait'
    });

    if (ap.anchorEnd) {
      wait.setAttribute('text-anchor', 'end');
    }

    wait.textContent = 'wait ' + road.waitingTime + 's';
    labelLayer.appendChild(wait);

    const hidden = road.vehicleCount - MAX_CARS_DRAWN;

    if (hidden > 0) {
      const more = svgEl('text', {
        x: ap.carX + ap.dx * (MAX_CARS_DRAWN * CAR_SLOT_GAP),
        y: ap.carY + ap.dy * (MAX_CARS_DRAWN * CAR_SLOT_GAP) + 4,
        class: 'road-wait',
        'text-anchor': 'middle'
      });

      more.textContent = '+' + hidden;
      labelLayer.appendChild(more);
    }
  });

  syncCars();

  if (reduceMotion) {
    APPROACHES.forEach(ap => {
      ap.queue.forEach((car, i) => {
        car.pos = i * CAR_SLOT_GAP;
        placeCar(ap, car);
      });
    });
  }
}

const DOT_COLOURS = [
  '#2fa060',
  '#d6b23e',
  '#e05a4e',
  '#4da3ff',
  '#9b7fe0',
  '#39b5a8'
];

function renderRoadControls() {
  const box = $('roadControls');

  if (box.querySelector('[data-editing]')) {
    return;
  }

  box.textContent = '';

  if (state.roads.length === 0) {
    const p = document.createElement('p');
    p.className = 'muted';
    p.textContent = 'No roads configured. Add one on the Roads tab.';
    box.appendChild(p);
    return;
  }

  state.roads.forEach((road, index) => {
    const row = document.createElement('div');
    row.className = 'rc' +
      (road.signalStatus === 'GREEN' ? ' is-green' : '');

    const name = document.createElement('div');
    name.className = 'rc-name';

    const dot = document.createElement('span');
    dot.className = 'rc-dot';
    dot.style.background = DOT_COLOURS[index % DOT_COLOURS.length];
    name.appendChild(dot);

    const label = document.createElement('span');
    label.textContent = road.roadName || road.roadID;
    name.appendChild(label);
    row.appendChild(name);

    const minus = document.createElement('button');
    minus.className = 'rc-btn';
    minus.textContent = '−';
    minus.title = 'One fewer vehicle';
    minus.addEventListener('click', () =>
      setCount(road.roadID, road.vehicleCount - 1)
    );
    row.appendChild(minus);

    const count = document.createElement('input');
    count.type = 'number';
    count.min = '0';
    count.className = 'rc-count';
    count.value = String(road.vehicleCount);
    count.title = 'Type an exact number and press Enter';

    count.addEventListener('keydown', e => {
      if (e.key === 'Enter') {
        e.target.blur();
      }
    });

    count.addEventListener('change', e => {
      const raw = e.target.value.trim();

      if (!/^\d+$/.test(raw)) {
        toast('Vehicles must be a whole number, 0 or more', 'bad');
        e.target.value = String(road.vehicleCount);
        return;
      }

      setCount(road.roadID, parseInt(raw, 10));
    });

    count.addEventListener('focus', () => {
      row.dataset.editing = '1';
    });

    count.addEventListener('blur', () => {
      delete row.dataset.editing;
    });

    row.appendChild(count);

    const plus = document.createElement('button');
    plus.className = 'rc-btn';
    plus.textContent = '+';
    plus.title = 'One more vehicle';
    plus.addEventListener('click', () =>
      setCount(road.roadID, road.vehicleCount + 1)
    );
    row.appendChild(plus);

    const emg = document.createElement('button');
    emg.className = 'rc-emg' +
      (road.emergencyStatus ? ' is-active' : '');

    emg.textContent = road.emergencyStatus ? 'Clear' : 'Send';

    emg.title = road.emergencyStatus
      ? 'Mark the emergency completed'
      : 'Send an emergency vehicle to this road';

    emg.addEventListener('click', () => {
      if (road.emergencyStatus) {
        endEmergency(road.roadID);
      } else {
        startEmergency(road.roadID);
      }
    });

    row.appendChild(emg);

    const sub = document.createElement('div');
    sub.className = 'rc-sub';
    sub.textContent = 'vehicles · ' + road.trafficDensity +
      '% density · priority ' + road.currentPriority;

    row.appendChild(sub);
    box.appendChild(row);
  });
}

function renderPriority() {
  const box = $('priorityBox');
  const p = state.priority;

  if (!p || !p.valid) {
    box.className = 'priority-empty';
    box.textContent = '';

    const note = document.createElement('p');
    note.className = 'muted';
    note.textContent = (p && (p.errorMessage || p.reason)) ||
      'No decision yet.';

    box.appendChild(note);
    return;
  }

  const emergency = /emergency/i.test(p.reason || '');

  box.className = 'priority-live' +
    (emergency ? ' is-emergency' : '');

  box.textContent = '';

  const name = document.createElement('p');
  name.className = 'road-name';
  name.textContent = p.roadName || p.roadID;
  box.appendChild(name);

  const meta = document.createElement('p');
  meta.className = 'road-meta';
  meta.textContent = p.roadID + ' · ' + p.signalStatus +
    ' · score ' + p.priority;

  box.appendChild(meta);

  const reason = document.createElement('p');
  reason.className = 'reason';
  reason.textContent = p.reason || '—';
  box.appendChild(reason);
}

function refreshSelects() {
  ['removeSelect', 'detailSelect', 'trafficSelect', 'histRoad']
    .forEach(id => {
      const select = $(id);
      if (!select) return;

      const previous = select.value;
      select.textContent = '';

      if (id === 'histRoad') {
        const all = document.createElement('option');
        all.value = '';
        all.textContent = 'All roads';
        select.appendChild(all);
      }

      state.roads.forEach(r => {
        const option = document.createElement('option');
        option.value = r.roadID;
        option.textContent = r.roadID + ' — ' + r.roadName;
        select.appendChild(option);
      });

      if (previous && state.roads.some(r => r.roadID === previous)) {
        select.value = previous;
      }
    });
}

function updateRoadDetail() {
  const select = $('detailSelect');
  const box = $('roadDetail');

  if (!select || !box) return;

  const road = state.roads.find(r => r.roadID === select.value);

  if (!road) {
    box.textContent = 'Select a road.';
    return;
  }

  box.textContent =
    'Road ID        : ' + road.roadID + '\n' +
    'Name           : ' + road.roadName + '\n' +
    'Vehicles       : ' + road.vehicleCount + '\n' +
    'Density        : ' + road.trafficDensity + '%\n' +
    'Waiting time   : ' + road.waitingTime + ' s\n' +
    'Emergency      : ' + (road.emergencyStatus ? 'ACTIVE' : 'none') + '\n' +
    'Priority score : ' + road.currentPriority + '\n' +
    'Signal         : ' + road.signalStatus + '\n' +
    'Active         : ' + (road.activeStatus ? 'yes' : 'no');
}

function renderAll() {
  $('simState').textContent = state.simulation;
  $('cycleBadge').textContent = 'cycle ' + state.cycle;

  renderJunction();
  renderRoadControls();
  renderPriority();
  refreshSelects();
  updateRoadDetail();

  $('btnPause').disabled = state.simulation !== 'RUNNING';
  $('btnResume').disabled = state.simulation !== 'PAUSED';
  $('btnStart').disabled = state.simulation === 'RUNNING';
}

function setConnected(value) {
  if (state.connected === value) return;

  state.connected = value;

  const pill = $('connection');
  pill.textContent = value ? 'connected' : 'offline';
  pill.className = 'pill ' + (value ? 'pill-ok' : 'pill-bad');

  $('offlineBanner').hidden = value;
}

async function api(method, path, body) {
  const options = {
    method,
    headers: {}
  };

  if (body !== undefined) {
    options.headers['Content-Type'] = 'application/json';
    options.body = JSON.stringify(body);
  }

  let response;

  try {
    response = await fetch(path, options);
  } catch (networkError) {
    setConnected(false);
    throw new Error('Cannot reach the server. Is traffic_server running?');
  }

  let payload;

  try {
    payload = await response.json();
  } catch (parseError) {
    throw new Error(
      'Server sent a malformed response (HTTP ' + response.status + ')'
    );
  }

  setConnected(true);

  if (!response.ok || payload.ok === false) {
    throw new Error(
      payload.error ||
      payload.message ||
      ('Request failed with HTTP ' + response.status)
    );
  }

  return payload;
}

async function poll() {
  try {
    const data = await api('GET', 'api/traffic/state');

    state.roads = data.roads || [];
    state.priority = data.priority || null;
    state.simulation = data.simulation || 'STOPPED';
    state.cycle = data.cycle || 0;

    if (data.cycleIntervalMs) {
      state.cycleIntervalMs = data.cycleIntervalMs;

      const sel = $('speedSelect');

      if (sel && document.activeElement !== sel) {
        sel.value = String(data.cycleIntervalMs);
      }
    }

    if ((!state.priority || !state.priority.valid) &&
        state.roads.length) {
      try {
        state.priority = await api('GET', 'api/priority');
      } catch (ignored) {
      }
    }

    const storage = $('storage');
    storage.textContent = 'storage ' + (data.storage || '—');
    storage.className = 'pill ' +
      (data.storage && data.storage !== 'none'
        ? 'pill-ok'
        : 'pill-wait');

    renderAll();
  } catch (err) {
    setConnected(false);
  }
}

function toast(message, kind) {
  const area = $('toastArea');
  const node = document.createElement('div');

  node.className = 'toast ' + (kind || 'info');
  node.textContent = message;

  area.appendChild(node);

  setTimeout(() => node.remove(), 3800);
}

function setFieldError(input, message) {
  if (!input) return;

  input.classList.toggle('invalid', Boolean(message));

  let slot = input.parentElement.querySelector('.field-error');

  if (!slot) {
    slot = document.createElement('span');
    slot.className = 'field-error';
    input.parentElement.appendChild(slot);
  }

  slot.textContent = message || '';
}

function clearErrors(container) {
  container.querySelectorAll('.invalid')
    .forEach(el => el.classList.remove('invalid'));

  container.querySelectorAll('.field-error')
    .forEach(el => {
      el.textContent = '';
    });
}

function readInt(input, opts) {
  const o = opts || {};
  const min = o.min === undefined ? 0 : o.min;
  const max = o.max === undefined ? Infinity : o.max;
  const required = o.required === undefined ? true : o.required;
  const label = o.label || 'Value';

  const raw = input.value.trim();

  if (raw === '') {
    if (!required) {
      setFieldError(input, '');
      return null;
    }

    setFieldError(input, label + ' is required');
    return undefined;
  }

  if (!/^-?\d+$/.test(raw)) {
    setFieldError(input, label + ' must be a whole number');
    return undefined;
  }

  const value = parseInt(raw, 10);

  if (value < min) {
    setFieldError(input, label + ' cannot be below ' + min);
    return undefined;
  }

  if (value > max) {
    setFieldError(input, label + ' cannot be above ' + max);
    return undefined;
  }

  setFieldError(input, '');
  return value;
}

async function nudge(roadID, add, remove) {
  try {
    await api('POST', 'api/traffic/update', {
      roadID,
      addVehicles: add,
      removeVehicles: remove
    });

    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function setCount(roadID, target) {
  if (target < 0) {
    target = 0;
  }

  try {
    await api('PUT', 'api/road/' + encodeURIComponent(roadID), {
      vehicleCount: target
    });

    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function setAllCounts(target) {
  try {
    for (const road of state.roads) {
      await api('PUT', 'api/road/' + encodeURIComponent(road.roadID), {
        vehicleCount: target
      });
    }

    toast(
      target === 0
        ? 'All roads emptied'
        : target + ' vehicle(s) on every road',
      'ok'
    );

    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function setSpeed(intervalMs) {
  try {
    await api('POST', 'api/simulation/speed', {
      intervalMs: intervalMs
    });
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function startEmergency(roadID) {
  try {
    const result = await api('POST', 'api/emergency/start', {
      roadID,
      type: 'Ambulance'
    });

    const chosen = result.priority && result.priority.roadID;

    toast(
      chosen
        ? 'Emergency active — GREEN to ' + chosen
        : (result.message || 'Emergency started'),
      'ok'
    );

    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function endEmergency(roadID) {
  try {
    const result = await api('POST', 'api/emergency/end', {
      roadID
    });

    toast(result.message || 'Emergency completed', 'ok');
    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function addRoad() {
  const panel = $('view-manage');
  clearErrors(panel);

  const idInput = $('addID');
  const nameInput = $('addName');
  const roadID = idInput.value.trim();
  const roadName = nameInput.value.trim();

  let bad = false;

  if (!roadID) {
    setFieldError(idInput, 'Road ID is required');
    bad = true;
  } else if (state.roads.some(r => r.roadID === roadID)) {
    setFieldError(idInput, 'A road with this ID already exists');
    bad = true;
  }

  if (!roadName) {
    setFieldError(nameInput, 'Road name is required');
    bad = true;
  }

  const vehicles = readInt($('addVehicles'), {
    label: 'Vehicles'
  });

  const density = readInt($('addDensity'), {
    max: 100,
    label: 'Density'
  });

  const waiting = readInt($('addWaiting'), {
    label: 'Waiting time'
  });

  if (
    bad ||
    vehicles === undefined ||
    density === undefined ||
    waiting === undefined
  ) {
    toast('Fix the highlighted fields', 'bad');
    return;
  }

  try {
    const result = await api('POST', 'api/road', {
      roadID,
      roadName,
      vehicleCount: vehicles,
      trafficDensity: density,
      waitingTime: waiting
    });

    toast(result.message || 'Road added', 'ok');

    idInput.value = '';
    nameInput.value = '';

    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function removeRoad() {
  const roadID = $('removeSelect').value;

  if (!roadID) {
    toast('Select a road first', 'bad');
    return;
  }

  try {
    const result = await api(
      'DELETE',
      'api/road/' + encodeURIComponent(roadID)
    );

    toast(result.message || 'Road removed', 'ok');
    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function updateTraffic() {
  const panel = $('view-manage');
  clearErrors(panel);

  const roadID = $('trafficSelect').value;

  if (!roadID) {
    toast('Select a road first', 'bad');
    return;
  }

  const add = readInt($('addVeh'), {
    label: 'Add vehicles'
  });

  const remove = readInt($('removeVeh'), {
    label: 'Remove vehicles'
  });

  const density = readInt($('newDensity'), {
    max: 100,
    required: false,
    label: 'Density'
  });

  const waiting = readInt($('newWaiting'), {
    required: false,
    label: 'Waiting time'
  });

  if (
    add === undefined ||
    remove === undefined ||
    density === undefined ||
    waiting === undefined
  ) {
    toast('Fix the highlighted fields', 'bad');
    return;
  }

  if (add === 0 && remove === 0 && density === null && waiting === null) {
    toast('Nothing to change', 'info');
    return;
  }

  const payload = {
    roadID,
    addVehicles: add,
    removeVehicles: remove
  };

  if (density !== null) {
    payload.trafficDensity = density;
  }

  if (waiting !== null) {
    payload.waitingTime = waiting;
  }

  try {
    const result = await api('POST', 'api/traffic/update', payload);

    toast(result.message || 'Traffic updated', 'ok');

    $('addVeh').value = '0';
    $('removeVeh').value = '0';

    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

let currentQueue = [];

async function loadQueue() {
  const roadID = $('detailSelect').value;
  const box = $('queueList');
  const note = $('vehicleResult');

  if (!roadID) {
    currentQueue = [];
    box.textContent = '';

    const p = document.createElement('p');
    p.className = 'muted tiny qempty';
    p.textContent = 'Select a road.';

    box.appendChild(p);
    note.textContent = '';

    return;
  }

  try {
    const data = await api(
      'GET',
      'api/queue?roadID=' + encodeURIComponent(roadID)
    );

    currentQueue = data.vehicles || [];
    renderQueue();

    const road = state.roads.find(r => r.roadID === roadID);

    if (road && road.vehicleCount > currentQueue.length) {
      note.textContent =
        road.vehicleCount + ' vehicles on this road, ' +
        currentQueue.length +
        ' tracked individually in the queue. ' +
        'Add one below to search for it.';
    } else {
      note.textContent = '';
    }
  } catch (err) {
    currentQueue = [];
    box.textContent = '';

    const p = document.createElement('p');
    p.className = 'muted tiny qempty';
    p.textContent = err.message;

    box.appendChild(p);
  }
}

function renderQueue() {
  const box = $('queueList');
  const term = $('queueFilter').value.trim().toLowerCase();

  box.textContent = '';

  const matches = term
    ? currentQueue.filter(v =>
        v.vehicleID.toLowerCase().includes(term) ||
        v.vehicleType.toLowerCase().includes(term)
      )
    : currentQueue;

  if (currentQueue.length === 0) {
    const p = document.createElement('p');
    p.className = 'muted tiny qempty';
    p.textContent = 'No vehicles tracked in this queue yet.';

    box.appendChild(p);
    return;
  }

  if (matches.length === 0) {
    const p = document.createElement('p');
    p.className = 'muted tiny qempty';
    p.textContent =
      'Nothing in this queue matches "' +
      $('queueFilter').value.trim() +
      '".';

    box.appendChild(p);
    return;
  }

  matches.forEach(v => {
    const row = document.createElement('div');

    row.className = 'qrow' +
      (v.emergency ? ' is-emg' : '') +
      (v.position === 1 ? ' is-next' : '') +
      (term ? ' is-hit' : '');

    const pos = document.createElement('span');
    pos.className = 'qpos';
    pos.textContent = '#' + v.position;

    const id = document.createElement('span');
    id.className = 'qid';
    id.textContent = v.vehicleID;

    const type = document.createElement('span');
    type.className = 'qtype';
    type.textContent = v.emergency
      ? v.vehicleType + ' · EMERGENCY'
      : v.vehicleType;

    row.appendChild(pos);
    row.appendChild(id);
    row.appendChild(type);

    box.appendChild(row);
  });
}

async function confirmPosition(vehicleID) {
  const roadID = $('detailSelect').value;

  if (!roadID || !vehicleID) return;

  try {
    const data = await api(
      'GET',
      'api/vehicle/find?roadID=' +
      encodeURIComponent(roadID) +
      '&vehicleID=' +
      encodeURIComponent(vehicleID)
    );

    $('vehicleResult').textContent = data.found
      ? vehicleID + ' is number ' + data.position +
        ' in the queue — ' + (data.position - 1) + ' ahead of it'
      : vehicleID + ' is not in this queue';
  } catch (err) {
    $('vehicleResult').textContent = err.message;
  }
}

function onQueueFilter() {
  renderQueue();

  const term = $('queueFilter').value.trim();

  const exact = currentQueue.find(
    v => v.vehicleID.toLowerCase() === term.toLowerCase()
  );

  if (exact) {
    confirmPosition(exact.vehicleID);
  } else if (term === '') {
    $('vehicleResult').textContent = '';
  }
}

async function queueVehicle() {
  const roadID = $('detailSelect').value;
  const input = $('newVehicleID');
  const vehicleID = input.value.trim();

  if (!roadID) {
    toast('Select a road first', 'bad');
    return;
  }

  if (!vehicleID) {
    setFieldError(input, 'Vehicle ID is required');
    return;
  }

  setFieldError(input, '');

  try {
    const result = await api('POST', 'api/queue', {
      roadID,
      vehicleID,
      type: $('newVehicleType').value
    });

    toast(result.message || 'Vehicle queued', 'ok');

    input.value = '';

    await poll();
    await loadQueue();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function simulation(action) {
  try {
    const result = await api('POST', 'api/simulation/' + action);

    toast(result.message || action, 'ok');
    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function stepCycle() {
  try {
    await api('POST', 'api/cycle');
    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function randomise() {
  if (state.roads.length === 0) {
    toast('No roads to randomise', 'bad');
    return;
  }

  try {
    for (const road of state.roads) {
      const target = Math.floor(Math.random() * 18);

      await api('POST', 'api/traffic/update', {
        roadID: road.roadID,
        addVehicles: Math.max(0, target - road.vehicleCount),
        removeVehicles: Math.max(0, road.vehicleCount - target)
      });
    }

    toast('Traffic randomised', 'ok');
    await poll();
  } catch (err) {
    toast(err.message, 'bad');
  }
}

async function loadHistory() {
  const kind = $('histKind').value;
  const roadID = $('histRoad').value;

  const limit = readInt($('histLimit'), {
    min: 1,
    max: 200,
    label: 'Limit'
  });

  if (limit === undefined) return;

  let url = 'api/history/' + kind + '?limit=' + limit;

  if (roadID) {
    url += '&roadID=' + encodeURIComponent(roadID);
  }

  const columns = {
    traffic: [
      'recordID',
      'roadID',
      'vehicleCount',
      'trafficDensity',
      'waitingTime',
      'timestamp'
    ],
    signal: [
      'recordID',
      'roadID',
      'previousSignal',
      'newSignal',
      'timestamp'
    ],
    emergency: [
      'eventID',
      'roadID',
      'emergencyType',
      'startTime',
      'endTime',
      'status'
    ]
  }[kind];

  try {
    const data = await api('GET', url);

    const table = $('historyTable');
    const head = table.querySelector('thead');
    const body = table.querySelector('tbody');

    head.textContent = '';
    body.textContent = '';

    const headRow = document.createElement('tr');

    columns.forEach(c => {
      const th = document.createElement('th');
      th.textContent = c;
      headRow.appendChild(th);
    });

    head.appendChild(headRow);

    const rows = data.records || [];

    if (rows.length === 0) {
      const tr = document.createElement('tr');
      const td = document.createElement('td');

      td.colSpan = columns.length;
      td.className = 'muted';
      td.textContent = 'No records.';

      tr.appendChild(td);
      body.appendChild(tr);

      return;
    }

    rows.forEach(record => {
      const tr = document.createElement('tr');

      columns.forEach(c => {
        const td = document.createElement('td');

        td.textContent =
          record[c] === undefined ? '' : String(record[c]);

        tr.appendChild(td);
      });

      body.appendChild(tr);
    });

    toast('Loaded ' + rows.length + ' record(s)', 'ok');
  } catch (err) {
    toast(err.message, 'bad');
  }
}

function setupTabs() {
  document.querySelectorAll('.vtab').forEach(tab => {
    tab.addEventListener('click', () => {
      document.querySelectorAll('.vtab')
        .forEach(t => t.classList.remove('is-active'));

      document.querySelectorAll('.view')
        .forEach(v => v.classList.remove('is-active'));

      tab.classList.add('is-active');

      $('view-' + tab.dataset.view).classList.add('is-active');

      if (tab.dataset.view === 'manage') {
        loadQueue();
      }
    });
  });
}

function setup() {
  drawScenery();
  setupTabs();

  $('btnStart').addEventListener('click', () => simulation('start'));
  $('btnPause').addEventListener('click', () => simulation('pause'));
  $('btnResume').addEventListener('click', () => simulation('resume'));
  $('btnReset').addEventListener('click', () => simulation('reset'));
  $('btnStep').addEventListener('click', stepCycle);
  $('btnRandom').addEventListener('click', randomise);

  $('btnClearAll').addEventListener('click', () => setAllCounts(0));
  $('btnOneEach').addEventListener('click', () => setAllCounts(1));

  $('speedSelect').addEventListener('change', e =>
    setSpeed(parseInt(e.target.value, 10))
  );

  $('btnAddRoad').addEventListener('click', addRoad);
  $('btnRemoveRoad').addEventListener('click', removeRoad);
  $('btnUpdateTraffic').addEventListener('click', updateTraffic);

  $('detailSelect').addEventListener('change', () => {
    updateRoadDetail();
    $('queueFilter').value = '';
    loadQueue();
  });

  $('queueFilter').addEventListener('input', onQueueFilter);
  $('btnQueueVehicle').addEventListener('click', queueVehicle);
  $('btnLoadHistory').addEventListener('click', loadHistory);

  renderJunction();

  if (!reduceMotion) {
    requestAnimationFrame(animate);
  }

  poll();
  pollTimer = setInterval(poll, POLL_MS);
}

document.addEventListener('DOMContentLoaded', setup);
