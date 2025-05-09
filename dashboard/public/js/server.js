const express = require('express');
const { InfluxDB } = require('influx');
const app = express();
const port = 3000;

app.use(express.json());

const influx = new InfluxDB({
    host: 'localhost',
    database: 'stedin',
    username: 'admin',
    password: 'yourpassword',
    schema: [
        {
            measurement: 'sensors',
            fields: { value: 'float' },
            tags: ['sensor_data']
        }
    ]
});

// Test connection
influx.ping(5000).then(hosts => {
    hosts.forEach(host => {
        if (host.online) {
            console.log(`Connected to InfluxDB at ${host.url.host}`);
        } else {
            console.error(`InfluxDB at ${host.url.host} is offline`);
        }
    });
}).catch(err => {
    console.error('Error connecting to InfluxDB:', err);
});

// API endpoint to write data
app.post('/api/write', async (req, res) => {
    const { temperature, humidity, power } = req.body;
    try {
        await influx.writePoints([
            {
                measurement: 'sensors',
                tags: { sensor_data: 'temperature' },
                fields: { value: temperature },
                timestamp: new Date()
            },
            {
                measurement: 'sensors',
                tags: { sensor_data: 'humidity' },
                fields: { value: humidity },
                timestamp: new Date()
            },
            {
                measurement: 'sensors',
                tags: { sensor_data: 'power' },
                fields: { value: power },
                timestamp: new Date()
            }
        ]);
        res.sendStatus(200);
    } catch (err) {
        console.error('Influx write error:', err);
        res.sendStatus(500);
    }
});

app.listen(port, () => console.log(`Server running on port ${port}`));
