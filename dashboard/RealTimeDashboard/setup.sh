mkdir -p templates
cat > requirements.txt << EOF
flask==2.3.3
flask-cors==4.0.0
influxdb-client==1.36.1
python-dotenv==1.0.0
EOF

cat > templates/index.html << EOF
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>InfluxDB Dashboard</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/bootstrap/5.3.0/css/bootstrap.min.css">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.9.1/chart.min.js"></script>
    <style>
        .chart-container {
            width: 100%;
            height: 400px;
            margin-bottom: 30px;
        }
        .card {
            margin-bottom: 20px;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
        }
        .chart-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 20px;
        }
        h1 {
            margin-bottom: 30px;
        }
    </style>
</head>
<body>
    <div class="container mt-5">
        <h1 class="text-center">Sensor Data Dashboard</h1>
        
        <div class="row">
            <div class="col-md-6">
                <div class="card">
                    <div class="chart-header">
                        <h3>Temperature</h3>
                        <div class="btn-group">
                            <button class="btn btn-sm btn-outline-secondary temp-range" data-hours="6">6h</button>
                            <button class="btn btn-sm btn-outline-secondary temp-range" data-hours="12">12h</button>
                            <button class="btn btn-sm btn-outline-secondary temp-range active" data-hours="24">24h</button>
                        </div>
                    </div>
                    <div class="card-body">
                        <div class="chart-container">
                            <canvas id="temperatureChart"></canvas>
                        </div>
                    </div>
                </div>
            </div>
            
            <div class="col-md-6">
                <div class="card">
                    <div class="chart-header">
                        <h3>Humidity</h3>
                        <div class="btn-group">
                            <button class="btn btn-sm btn-outline-secondary humidity-range" data-hours="6">6h</button>
                            <button class="btn btn-sm btn-outline-secondary humidity-range" data-hours="12">12h</button>
                            <button class="btn btn-sm btn-outline-secondary humidity-range active" data-hours="24">24h</button>
                        </div>
                    </div>
                    <div class="card-body">
                        <div class="chart-container">
                            <canvas id="humidityChart"></canvas>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script>
        // Global chart objects
        let temperatureChart;
        let humidityChart;

        // Chart colors
        const tempColor = 'rgba(255, 99, 132, 0.7)';
        const humidityColor = 'rgba(54, 162, 235, 0.7)';

        // Fetch temperature data and create chart
        async function fetchTemperatureData(hours = 24) {
            try {
                const response = await fetch('/api/temperature?hours=' + hours);
                const data = await response.json();
                return data;
            } catch (error) {
                console.error('Error fetching temperature data:', error);
                return [];
            }
        }

        // Fetch humidity data and create chart
        async function fetchHumidityData(hours = 24) {
            try {
                const response = await fetch('/api/humidity?hours=' + hours);
                const data = await response.json();
                return data;
            } catch (error) {
                console.error('Error fetching humidity data:', error);
                return [];
            }
        }

        // Initialize temperature chart
        async function initTemperatureChart() {
            const data = await fetchTemperatureData();
            const ctx = document.getElementById('temperatureChart').getContext('2d');
            
            temperatureChart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: data.map(item => item.time),
                    datasets: [{
                        label: 'Temperature',
                        data: data.map(item => item.value),
                        borderColor: tempColor,
                        backgroundColor: tempColor,
                        tension: 0.1,
                        pointRadius: 2,
                        borderWidth: 2,
                        fill: false
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        y: {
                            title: {
                                display: true,
                                text: 'Temperature (°C)'
                            }
                        },
                        x: {
                            title: {
                                display: true,
                                text: 'Time'
                            }
                        }
                    }
                }
            });
        }

        // Initialize humidity chart
        async function initHumidityChart() {
            const data = await fetchHumidityData();
            const ctx = document.getElementById('humidityChart').getContext('2d');
            
            humidityChart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: data.map(item => item.time),
                    datasets: [{
                        label: 'Humidity',
                        data: data.map(item => item.value),
                        borderColor: humidityColor,
                        backgroundColor: humidityColor,
                        tension: 0.1,
                        pointRadius: 2,
                        borderWidth: 2,
                        fill: false
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        y: {
                            title: {
                                display: true,
                                text: 'Humidity (%)'
                            },
                            min: 0,
                            max: 100
                        },
                        x: {
                            title: {
                                display: true,
                                text: 'Time'
                            }
                        }
                    }
                }
            });
        }

        // Update temperature chart with new data
        async function updateTemperatureChart(hours) {
            const data = await fetchTemperatureData(hours);
            temperatureChart.data.labels = data.map(item => item.time);
            temperatureChart.data.datasets[0].data = data.map(item => item.value);
            temperatureChart.update();
        }

        // Update humidity chart with new data
        async function updateHumidityChart(hours) {
            const data = await fetchHumidityData(hours);
            humidityChart.data.labels = data.map(item => item.time);
            humidityChart.data.datasets[0].data = data.map(item => item.value);
            humidityChart.update();
        }

        // Initialize both charts when the page loads
        document.addEventListener('DOMContentLoaded', function() {
            initTemperatureChart();
            initHumidityChart();
            
            // Add event listeners for temperature range buttons
            document.querySelectorAll('.temp-range').forEach(button => {
                button.addEventListener('click', function() {
                    document.querySelectorAll('.temp-range').forEach(btn => btn.classList.remove('active'));
                    this.classList.add('active');
                    const hours = this.getAttribute('data-hours');
                    updateTemperatureChart(hours);
                });
            });
            
            // Add event listeners for humidity range buttons
            document.querySelectorAll('.humidity-range').forEach(button => {
                button.addEventListener('click', function() {
                    document.querySelectorAll('.humidity-range').forEach(btn => btn.classList.remove('active'));
                    this.classList.add('active');
                    const hours = this.getAttribute('data-hours');
                    updateHumidityChart(hours);
                });
            });
        });
    </script>
</body>
</html>
EOF

cat > app.py << EOF
from flask import Flask, jsonify, render_template, request
from flask_cors import CORS
import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS
from datetime import datetime, timedelta
import os

app = Flask(__name__)
CORS(app, resources={r"/api/*": {"origins": ["http://127.0.0.1:5500", "http://localhost:5500"]}})


# InfluxDB configuration
INFLUX_URL = "http://localhost:8086"
TOKEN = "QNU9pj1aTm-fipRW9ZkU5eYvfdAOfVC7pwhX5jdN-lTsx6ZluEIyQyn38oSgRXdG2SSGuVxwnWxPCRFC5wxNvg=="
ORG = "stedin"
BUCKET = "data"

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/temperature', methods=['GET'])
def get_temperature_data():
    # Get time range parameter (default to 24 hours)
    hours = request.args.get('hours', default=24, type=int)
    
    client = influxdb_client.InfluxDBClient(
        url=INFLUX_URL,
        token=TOKEN,
        org=ORG
    )
    
    # Time range
    end_time = datetime.utcnow()
    start_time = end_time - timedelta(hours=hours)
    
    # Create query api
    query_api = client.query_api()
    
    # Query temperature data
    query = f'''
    from(bucket: "{BUCKET}")
      |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {end_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
      |> filter(fn: (r) => r._measurement == "temperature")
      |> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value")
    '''
    
    result = query_api.query(query=query, org=ORG)
    
    # Process the results to a format suitable for charts
    data = []
    for table in result:
        for record in table.records:
            time = record.get_time().strftime("%Y-%m-%d %H:%M:%S")
            value = record.get_value()
            if value is not None:
                data.append({"time": time, "value": value})
    
    return jsonify(data)

@app.route('/api/humidity', methods=['GET'])
def get_humidity_data():
    # Get time range parameter (default to 24 hours)
    hours = request.args.get('hours', default=24, type=int)
    
    client = influxdb_client.InfluxDBClient(
        url=INFLUX_URL,
        token=TOKEN,
        org=ORG
    )
    
    # Time range
    end_time = datetime.utcnow()
    start_time = end_time - timedelta(hours=hours)
    
    # Create query api
    query_api = client.query_api()
    
    # Query humidity data
    query = f'''
    from(bucket: "{BUCKET}")
      |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {end_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
      |> filter(fn: (r) => r._measurement == "humidity")
      |> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value")
    '''
    
    result = query_api.query(query=query, org=ORG)
    
    # Process the results to a format suitable for charts
    data = []
    for table in result:
        for record in table.records:
            time = record.get_time().strftime("%Y-%m-%d %H:%M:%S")
            value = record.get_value()
            if value is not None:
                data.append({"time": time, "value": value})
    
    return jsonify(data)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
EOF

echo "Project setup complete!"
echo "1. Install requirements: pip install -r requirements.txt"
echo "2. Run the app: python app.py"
echo "3. Open http://localhost:5000 in your browser"