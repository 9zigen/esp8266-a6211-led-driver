const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const app = express();
const bodyParser = require('body-parser');

//initialize a simple http server
const server = http.createServer(app);

// create application/json parser
var jsonParser = bodyParser.json()

var token;

app.get('/', function (req, res) {
  res.send('Hello World');
});

app.post('/auth', jsonParser, function(req, res) {
  if (!req.body) return res.sendStatus(400)

  if (req.body.login === "login" && req.body.password === "password") {
    token = Math.random();
    res.json({'success':true, 'token':token});
  }
  else
    res.json({'success':false});

  console.log(req.body);
  // console.log(req.text.pass);
});

// Enable CORS on ExpressJS to avoid cross-origin errors when calling this server using AJAX
// We are authorizing all domains to be able to manage information via AJAX (this is just for development)

app.use(function(req, res, next) {
  // Instead of "*" you should enable only specific origins
  res.header('Access-Control-Allow-Origin', '*');
  // Supported HTTP verbs
  res.header('Access-Control-Allow-Methods', 'GET,PUT,POST,DELETE');
  // Other custom headers
  res.header('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept');
  next();
});

// Body parser middleware to auto-parse request text to JSON
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

const wss = new WebSocket.Server({
    server,
    path: "/ws",
});


let networks = [
    {
      id: 0,
      ssid: "wifi 1",
      password: "12345678",
      dhcp: false,
      ip: "192.168.1.4",
      mask: "255.255.255.0",
      gateway: "192.168.1.1",
      dns: "192.168.1.1"
    },
    {
      id: 1,
      ssid: "wifi 2",
      password: "111111111",
      dhcp: true,
      ip: "",
      mask: "",
      gateway: "",
      dns: ""
    }
  ];

let leds = [
  {
    id: 0,
    color: '#DDEFFF',
    default_duty: 0
  },
  {
    id: 1,
    color: '#DDEFFF',
    default_duty: 20
  },
  {
    id: 2,
    color: '#DDEFFF',
    default_duty: 0
  }
];

let schedule = [
  {
    time_hour: 9,
    time_minute: 0,
    enabled: true,
    duty: [
      10, 20, 10
    ]
  },
  {
    time_hour: 12,
    time_minute: 0,
    enabled: false,
    duty: [
      40, 20, 10
    ]
  },
  {
    time_hour: 13,
    time_minute: 0,
    enabled: true,
    duty: [
      100, 100, 100
    ]
  },
  {
    time_hour: 19,
    time_minute: 0,
    enabled: true,
    duty: [
      0, 10, 0
    ]
  }

];

let status = {
  upTime: 360,
  chipId: 1827,
  freeHeap: 23567,
  vcc: 20,
  wifiMode: 'STA',
  ipAddress: '192.168.1.199',
  macAddress: '0A:EE:00:00:01:90',
  dhcp: true
}


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
