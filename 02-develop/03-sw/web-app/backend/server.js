const express = require('express');
const http = require('http');
const mqtt = require('mqtt');
const WebSocket = require('ws');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// MQTT Broker Details
const MQTT_BROKER = 'mqtt://192.168.1.45';
const MQTT_TOPICS = [
    'smart-garden/controller/status',
    'smart-garden/valve/+/state',
    'smart-garden/schedule/state'
];

// Store last known states for new WebSocket connections
let lastControllerStatus = 'unknown';
let lastScheduleState = JSON.stringify({
    enabled: true,
    programs: []
});

// Connect to MQTT Broker
const client = mqtt.connect(MQTT_BROKER);

client.on('connect', () => {
    console.log('Connected to MQTT Broker');
    client.subscribe(MQTT_TOPICS, (err) => {
        if (!err) {
            console.log('Subscribed to topics:', MQTT_TOPICS);
        } else {
            console.error('MQTT Subscription Error:', err);
        }
    });
});

client.on('message', (topic, message) => {
    console.log(`MQTT Message Received: Topic='${topic}', Message='${message.toString()}'`);

    // Update last known states
    if (topic === 'smart-garden/controller/status') {
        lastControllerStatus = message.toString();
    } else if (topic === 'smart-garden/schedule/state') {
        lastScheduleState = message.toString();
    }

    // Forward MQTT messages to all connected WebSocket clients
    wss.clients.forEach(ws => {
        if (ws.readyState === WebSocket.OPEN) {
            const data = { topic, message: message.toString() };
            ws.send(JSON.stringify(data));
        }
    });
});

// WebSocket connection handling
wss.on('connection', (ws) => {
    console.log('Client connected to WebSocket');

    // Send current known states to the newly connected client
    ws.send(JSON.stringify({ topic: 'smart-garden/controller/status', message: lastControllerStatus }));
    ws.send(JSON.stringify({ topic: 'smart-garden/schedule/state', message: lastScheduleState }));

    ws.on('close', () => {
        console.log('Client disconnected from WebSocket');
    });
});

// API endpoint to send commands to the ESP32
app.use(express.json());
app.post('/api/command', (req, res) => {
    const { topic, message } = req.body;
    console.log(`API Command Received: Topic='${topic}', Message='${message}'`);
    // Ensure topic is provided and message is a string (can be empty)
    if (topic && typeof message === 'string') {
        client.publish(topic, message, { qos: 1 }, (err) => {
            if (err) {
                console.error('MQTT Publish Error:', err);
                return res.status(500).send({ status: 'error', message: 'Failed to publish command' });
            }
            console.log(`MQTT Published: Topic='${topic}', Message='${message}'`);
            res.send({ status: 'success', message: 'Command published' });
        });
    } else {
        res.status(400).send({ status: 'error', message: 'Invalid command: topic must be provided and message must be a string.' });
    }
});

const PORT = process.env.PORT || 3001;
server.listen(PORT, () => {
    console.log(`Backend server is running on http://localhost:${PORT}`);
});