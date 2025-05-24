from flask import Flask, jsonify, render_template, request
from flask_cors import CORS
import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS
from datetime import datetime, timedelta, timezone
import os
import logging

# Set up logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

app = Flask(__name__)
# Enable CORS for all origins to work with Live Server
CORS(app, resources={r"/api/*": {"origins": "*"}})

# InfluxDB configuration
INFLUX_URL = "http://localhost:8086"
TOKEN = "8eYfRmBQIkGmfYFewISeBXNv1d5PItUFyK3i5ZWeqEUy-o7zrvsREIz4o9PlGGdVu2yoEJiFbvKIanhwJyeYag=="
ORG = "stedin"
BUCKET = "data"

# Fixed time range - Update these values as needed
FIXED_START_TIME = datetime(2025, 5, 18, 17, 30, 0, tzinfo=timezone.utc)
FIXED_END_TIME = datetime(2025, 5, 18, 20, 0, 0, tzinfo=timezone.utc)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/timerange', methods=['GET'])
def get_time_range():
    """Return the fixed time range for the frontend to use"""
    return jsonify({
        "start": FIXED_START_TIME.strftime("%Y-%m-%d %H:%M:%S"),
        "end": FIXED_END_TIME.strftime("%Y-%m-%d %H:%M:%S")
    })

@app.route('/api/temperature', methods=['GET'])
def get_temperature_data():
    try:
        logger.debug(f"Fetching temperature data from {FIXED_START_TIME} to {FIXED_END_TIME}")
        
        client = influxdb_client.InfluxDBClient(
            url=INFLUX_URL,
            token=TOKEN,
            org=ORG
        )
        
        query_api = client.query_api()
        
        # Try different query styles
        queries = [
            # Try as field
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {FIXED_START_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {FIXED_END_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._field == "temperature")
            ''',
            # Try as measurement
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {FIXED_START_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {FIXED_END_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._measurement == "temperature")
            ''',
            # Try with partial matching
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {FIXED_START_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {FIXED_END_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._field =~ /.*temp.*/ or r._measurement =~ /.*temp.*/)
            '''
        ]
        
        data = []
        for query in queries:
            logger.debug(f"Trying query: {query}")
            result = query_api.query(query=query, org=ORG)
            
            query_data = []
            for table in result:
                for record in table.records:
                    time = record.get_time().strftime("%Y-%m-%d %H:%M:%S")
                    value = record.get_value()
                    if value is not None:
                        query_data.append({"time": time, "value": value})
            
            logger.debug(f"Found {len(query_data)} data points with this query")
            
            if query_data:
                data = query_data
                break
        
        # If no data, return sample data for testing
        if len(data) == 0:
            logger.warning("No temperature data found, returning sample data")
            sample_data = generate_sample_temperature_data_fixed()
            return jsonify(sample_data)
        
        return jsonify(data)
    
    except Exception as e:
        logger.error(f"Error in temperature endpoint: {str(e)}", exc_info=True)
        return jsonify({"error": str(e)}), 500

@app.route('/api/humidity', methods=['GET'])
def get_humidity_data():
    try:
        logger.debug(f"Fetching humidity data from {FIXED_START_TIME} to {FIXED_END_TIME}")
        
        client = influxdb_client.InfluxDBClient(
            url=INFLUX_URL,
            token=TOKEN,
            org=ORG
        )
        
        query_api = client.query_api()
        
        # Try different query styles
        queries = [
            # Try as field
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {FIXED_START_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {FIXED_END_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._field == "humidity")
            ''',
            # Try as measurement
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {FIXED_START_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {FIXED_END_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._measurement == "humidity")
            ''',
            # Try with partial matching
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {FIXED_START_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {FIXED_END_TIME.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._field =~ /.*humid.*/ or r._measurement =~ /.*humid.*/)
            '''
        ]
        
        data = []
        for query in queries:
            logger.debug(f"Trying query: {query}")
            result = query_api.query(query=query, org=ORG)
            
            query_data = []
            for table in result:
                for record in table.records:
                    time = record.get_time().strftime("%Y-%m-%d %H:%M:%S")
                    value = record.get_value()
                    if value is not None:
                        query_data.append({"time": time, "value": value})
            
            logger.debug(f"Found {len(query_data)} data points with this query")
            
            if query_data:
                data = query_data
                break
        
        # If no data, return sample data for testing
        if len(data) == 0:
            logger.warning("No humidity data found, returning sample data")
            sample_data = generate_sample_humidity_data_fixed()
            return jsonify(sample_data)
        
        return jsonify(data)
    
    except Exception as e:
        logger.error(f"Error in humidity endpoint: {str(e)}", exc_info=True)
        return jsonify({"error": str(e)}), 500

# Generate sample temperature data for the fixed time range
def generate_sample_temperature_data_fixed():
    import random
    data = []
    
    # Calculate number of 5-minute intervals in the time range
    delta_minutes = int((FIXED_END_TIME - FIXED_START_TIME).total_seconds() / 60)
    intervals = delta_minutes // 5  # 5-minute intervals
    
    for i in range(intervals + 1):  # +1 to include the end time
        timestamp = FIXED_START_TIME + timedelta(minutes=5 * i)
        if timestamp > FIXED_END_TIME:
            break
        
        # Random temperature around 20°C with a slight trend
        base_temp = 20 + (i / intervals) * 2  # Slight increase over time
        value = base_temp + random.uniform(-1, 1)
        
        data.append({
            "time": timestamp.strftime("%Y-%m-%d %H:%M:%S"),
            "value": round(value, 1)
        })
    
    return data

# Generate sample humidity data for the fixed time range
def generate_sample_humidity_data_fixed():
    import random
    data = []
    
    # Calculate number of 5-minute intervals in the time range
    delta_minutes = int((FIXED_END_TIME - FIXED_START_TIME).total_seconds() / 60)
    intervals = delta_minutes // 5  # 5-minute intervals
    
    for i in range(intervals + 1):  # +1 to include the end time
        timestamp = FIXED_START_TIME + timedelta(minutes=5 * i)
        if timestamp > FIXED_END_TIME:
            break
        
        # Random humidity around 50% with a slight trend
        base_humidity = 50 - (i / intervals) * 5  # Slight decrease over time
        value = base_humidity + random.uniform(-3, 3)
        
        data.append({
            "time": timestamp.strftime("%Y-%m-%d %H:%M:%S"),
            "value": round(value, 1)
        })
    
    return data

@app.route('/api/status', methods=['GET'])
def get_status():
    return jsonify({
        "status": "ok",
        "fixed_time_range": {
            "start": FIXED_START_TIME.strftime("%Y-%m-%d %H:%M:%S"),
            "end": FIXED_END_TIME.strftime("%Y-%m-%d %H:%M:%S")
        }
    })

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')