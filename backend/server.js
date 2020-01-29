const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const app = express();
const cors = require('cors');
const bodyParser = require('body-parser');

//initialize a simple http server
const server = http.createServer(app);

// create application/json parser
var jsonParser = bodyParser.json();

var token;

// Enable CORS on ExpressJS to avoid cross-origin errors when calling this server using AJAX
// We are authorizing all domains to be able to manage information via AJAX (this is just for development)
app.use(cors());
app.options('*', cors()); // include before other routes

// Body parser middleware to auto-parse request text to JSON
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

/* Home page */
app.get('/', function (req, res) {
  res.send('Hello World');
});

/* On-lne Status */
app.get('/status', function (req, res) {
  res.send(JSON.stringify(status));
});

/* Control channels */
app.post('/duty', function (req, res) {
  duty = req.body.duty
  res.send(JSON.stringify(duty));
});

app.post('/brightness', function (req, res) {
  brightness = req.body.brightness
  res.send(JSON.stringify(brightness));
});

/* Schedule ----> */
app.get('/schedule', function (req, res) {
  res.send(JSON.stringify(schedule));
});

app.post('/schedule', function (req, res) {
  console.log(req.body);
  schedule = req.body.schedule
  res.send(JSON.stringify(schedule));
});


/* Settings ----> */
app.get('/leds', function (req, res) {
  res.send(JSON.stringify(leds));
});

app.post('/leds', function (req, res) {
  console.log(req.body);
  leds = req.body.leds
  res.send(JSON.stringify(leds));
});

app.get('/networks', function (req, res) {
  res.send(JSON.stringify(networks));
});

app.post('/networks', function (req, res) {
  networks = req.body.networks
  res.send(JSON.stringify(networks));
});

app.get('/services', function (req, res) {
  res.send(JSON.stringify(services));
});

app.post('/services', function (req, res) {
  services = req.body.services
  res.send(JSON.stringify(services));
});

app.get('/time', function (req, res) {
  res.send(JSON.stringify(time));
});

app.post('/time', function (req, res) {
  time = req.body.time
  res.send(JSON.stringify(time));
});

app.post('/auth', jsonParser, function(req, res) {
  if (!req.body) return res.sendStatus(400);

  if (req.body.login === "login" && req.body.password === "password") {
    token = Math.random();
    res.json({'success':true, 'token':token});
  }
  else
    res.json({'success':false});

  console.log(req.body);
  // console.log(req.text.pass);
});


const wss = new WebSocket.Server({
  server,
  path: "/ws",
});

let services = {
  hostname: 'test',
  ntp_server_name: '',
  utc_offset_minutes: 0,
  ntp_dst: true,
  mqtt_ip_address: '',
  mqtt_port: '',
  mqtt_user: '',
  mqtt_password: '',
  mqtt_qos: 0,
  enable_ntp_service: false,
  enable_mqtt_service: false
}

let networks = [
  {
    id: 0,
    ssid: 'Best WiFi',
    password: '',
    ip_address: '192.168.1.100',
    mask: '255.255.255.0',
    gateway: '192.168.1.1',
    dns: '192.168.1.1',
    dhcp: false
  },
  {
    id: 1,
    ssid: 'Best WiFi 2',
    password: '',
    ip_address: '',
    mask: '',
    gateway: '',
    dns: '',
    dhcp: true
  }
];

let leds = [
  {
    id: 0,
    color: '#DDEFFF',
    power: 50,
    state: 1
  },
  {
    id: 1,
    color: '#DDEFFF',
    power: 30,
    state: 1
  },
  {
    id: 2,
    color: '#DDEFFF',
    power: 40,
    state: 1
  },
  {
    id: 3,
    color: '#DDEFFF',
    power: 40,
    state: 1
  },
  {
    id: 4,
    color: '#DDEFFF',
    power: 40,
    state: 1
  }
];

let schedule = [
  {
    time_hour: 9,
    time_minute: 0,
    brightness: 50,
    duty: [
      10, 20, 10, 20, 20
    ]
  },
  {
    time_hour: 12,
    time_minute: 0,
    brightness: 100,
    duty: [
      40, 20, 10, 20, 20
    ]
  },
  {
    time_hour: 13,
    time_minute: 0,
    brightness: 120,
    duty: [
      100, 100, 100, 20, 20
    ]
  },
  {
    time_hour: 19,
    time_minute: 0,
    brightness: 20,
    duty: [
      0, 10, 0, 20, 20
    ]
  }

];

let status = {
  upTime: '1 day',
  localTime: '12:22',
  chipId: 1827,
  freeHeap: 23567,
  vcc: 20,
  wifiMode: 'STA',
  ipAddress: '192.168.1.199',
  macAddress: '0A:EE:00:00:01:90',
  mqttService: '',
  ntpService: '',
  brightness: 90,
  channels: [50, 100, 100, 20, 20]
};

let time = {
  year: 20,
  month: 10,
  weekday: 1,
  day: 10,
  hour: 12,
  minute: 1,
  second: 1,
  dst: 0,
  utc: 1
}

let duty = [50, 100, 100, 20, 20];

let brightness = 100;

function getRandomArbitrary(min, max) {
  return Math.random() * (max - min) + min;
}

function getRandomInt(min, max) {
  return Math.floor(Math.random() * (max - min)) + min;
}


function noop() {}

function heartbeat() {
  this.isAlive = true;
}

wss.on('connection', function connection(ws) {
  ws.isAlive = true;
  ws.on('pong', heartbeat);

  ws.on('message', function incoming(message) {
    console.log('received: %s', message);

    let json = JSON.parse(message);
    if (json.command !== undefined) {
      switch (json.command) {
        case ('getStatus'):
          ws.send(JSON.stringify({ status: status }));
          console.log(status);
          break;

        case ('getSettings'):
          ws.send(JSON.stringify({ leds: leds, schedule: schedule }));
          console.log(leds, schedule);
          break;

        case ('saveSettings'):
          // save led color
          leds = json.leds;
          // save schedule
          schedule = json.schedule;
          ws.send(JSON.stringify({ response: 'success' }));
          break;

        case ('getLeds'):
          ws.send(JSON.stringify({ leds: leds }));
          console.log(schedule);
          break;

        case ('setLeds'):
          leds = json.leds;
          ws.send(JSON.stringify({ response: 'success' }));
          break;

        case ('getSchedule'):
          ws.send(JSON.stringify({ schedule: schedule, capacity: 10 }));
          console.log(schedule);
          break;

        case ('setSchedule'):
          schedule = json.schedule;
          ws.send(JSON.stringify({ response: 'success' }));
          break;

        case ('getNetworks'):
          ws.send(JSON.stringify({ networks: networks, capacity: 2 }));
          console.log(schedule);
          break;

        case ('setNetworks'):
          networks = json.networks;
          ws.send(JSON.stringify({ response: 'success' }));
          break;


        default:
          ws.send(Date.now());
          break;
      }

    }

    // setTimeout(function timeout() {
    //   ws.send(Date.now());
    // }, 500);

  });

  // ws.send('something');
});

console.log("Started");

const interval = setInterval(function ping() {
  wss.clients.forEach(function each(ws) {
    if (ws.isAlive === false) return ws.terminate();

    ws.isAlive = false;
    ws.ping(noop);
  });
}, 30000);

function broadcast(data) {
  wss.clients.forEach(function each(client) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(data);
    }
  });
}

//start our server
server.listen(8081, () => {
  console.log(`Server started on port ${server.address().port} :)`);
});
