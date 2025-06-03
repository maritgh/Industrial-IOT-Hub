from flask import Flask, jsonify, render_template, request
from flask_cors import CORS
import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS
from datetime import datetime, timedelta, timezone
import os
import logging
import random

# Set up logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

app = Flask(__name__)
# Enable CORS for all origins to work with Live Server
CORS(app, resources={r"/api/*": {"origins": "*"}})

# InfluxDB configuration
INFLUX_URL = "http://host.docker.internal:8086"
TOKEN = "8eYfRmBQIkGmfYFewISeBXNv1d5PItUFyK3i5ZWeqEUy-o7zrvsREIz4o9PlGGdVu2yoEJiFbvKIanhwJyeYag=="
ORG = "stedin"
BUCKET = "data"

def parse_time_range(range_str):
    """Parse time range string to get start and end datetime objects"""
    now = datetime.now(timezone.utc)
    
    if range_str == '5m':
        start_time = now - timedelta(minutes=5)
    elif range_str == '1h':
        start_time = now - timedelta(hours=1)
    elif range_str == '3h':
        start_time = now - timedelta(hours=3)
    elif range_str == '24h':
        start_time = now - timedelta(hours=24)
    else:
        # Default to last 5 minutes
        start_time = now - timedelta(minutes=5)
    
    return start_time, now

def get_latest_status_code():
    """
    Get the latest status code from InfluxDB
    Returns the most recent status code value or None if not found
    """
    try:
        client = influxdb_client.InfluxDBClient(
            url=INFLUX_URL,
            token=TOKEN,
            org=ORG
        )
        
        query_api = client.query_api()
        
        # Try different query styles to find status code data
        queries = [
            # Try as field
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: -1h)
              |> filter(fn: (r) => r._field == "status_code" or r._field == "status")
              |> sort(columns: ["_time"], desc: true)
              |> limit(n: 1)
            ''',
            # Try as measurement
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: -1h)
              |> filter(fn: (r) => r._measurement == "status_code" or r._measurement == "status")
              |> sort(columns: ["_time"], desc: true)
              |> limit(n: 1)
            ''',
            # Try with partial matching
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: -1h)
              |> filter(fn: (r) => r._field =~ /.*status.*/ or r._measurement =~ /.*status.*/)
              |> sort(columns: ["_time"], desc: true)
              |> limit(n: 1)
            '''
        ]
        
        for query in queries:
            logger.debug(f"Trying status query: {query}")
            result = query_api.query(query=query, org=ORG)
            
            for table in result:
                for record in table.records:
                    value = record.get_value()
                    timestamp = record.get_time()
                    logger.debug(f"Found status value: {value} at {timestamp}")
                    return float(value) if value is not None else None
        
        logger.warning("No status code found in InfluxDB")
        return None
        
    except Exception as e:
        logger.error(f"Error fetching status code: {str(e)}")
        return None

def check_system_status():
    """
    Check system status based on status code from InfluxDB.
    Returns: dict with status, message, and optional details
    """
    try:
        # Get the latest status code from InfluxDB
        status_code = get_latest_status_code()
        
        if status_code is None:
            return {
                'status': 'error',
                'message': 'No status data available',
                'details': 'Unable to retrieve status code from InfluxDB',
                'status_code': None
            }
        
        # Check if status code indicates system is OK (1.11) or not (0)
        if abs(status_code - 1.11) < 0.01:  # Using small tolerance for float comparison
            return {
                'status': 'ok',
                'message': 'System operational',
                'details': f'Status code: {status_code} - All systems running normally',
                'status_code': status_code
            }
        elif abs(status_code - 0) < 0.01:  # Status code is 0
            return {
                'status': 'error',
                'message': 'System error detected',
                'details': f'Status code: {status_code} - System reporting error state',
                'status_code': status_code
            }
        else:
            # Unknown status code
            return {
                'status': 'error',
                'message': 'Unknown status code',
                'details': f'Unexpected status code: {status_code}',
                'status_code': status_code
            }
            
    except Exception as e:
        logger.error(f"Error checking system status: {str(e)}")
        return {
            'status': 'error',
            'message': 'Status check failed',
            'details': str(e),
            'status_code': None
        }

def check_influxdb_connection():
    """Check if InfluxDB is accessible"""
    try:
        client = influxdb_client.InfluxDBClient(
            url=INFLUX_URL,
            token=TOKEN,
            org=ORG
        )
        
        # Try a simple query to test connection
        query_api = client.query_api()
        query = f'buckets() |> filter(fn: (r) => r.name == "{BUCKET}") |> limit(n:1)'
        result = query_api.query(query=query, org=ORG)
        
        # If we get here without exception, connection is good
        return True
    except Exception as e:
        logger.error(f"InfluxDB connection check failed: {str(e)}")
        return False

def check_temperature_readings():
    """Check if we're getting recent temperature readings"""
    try:
        # Check for temperature data in the last 10 minutes
        now = datetime.now(timezone.utc)
        start_time = now - timedelta(minutes=10)
        
        client = influxdb_client.InfluxDBClient(
            url=INFLUX_URL,
            token=TOKEN,
            org=ORG
        )
        
        query_api = client.query_api()
        query = f'''
        from(bucket: "{BUCKET}")
          |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
          |> filter(fn: (r) => r._field == "temperature" or r._measurement == "temperature")
          |> limit(n: 1)
        '''
        
        result = query_api.query(query=query, org=ORG)
        
        # Check if we got any results
        for table in result:
            for record in table.records:
                return True
                
        # No recent temperature data found
        logger.warning("No recent temperature data found")
        return False
        
    except Exception as e:
        logger.error(f"Temperature readings check failed: {str(e)}")
        return False

def check_humidity_readings():
    """Check if we're getting recent humidity readings"""
    try:
        # Check for humidity data in the last 10 minutes
        now = datetime.now(timezone.utc)
        start_time = now - timedelta(minutes=10)
        
        client = influxdb_client.InfluxDBClient(
            url=INFLUX_URL,
            token=TOKEN,
            org=ORG
        )
        
        query_api = client.query_api()
        query = f'''
        from(bucket: "{BUCKET}")
          |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
          |> filter(fn: (r) => r._field == "humidity" or r._measurement == "humidity")
          |> limit(n: 1)
        '''
        
        result = query_api.query(query=query, org=ORG)
        
        # Check if we got any results
        for table in result:
            for record in table.records:
                return True
                
        # No recent humidity data found
        logger.warning("No recent humidity data found")
        return False
        
    except Exception as e:
        logger.error(f"Humidity readings check failed: {str(e)}")
        return False

def check_data_freshness():
    """Check if data is recent enough (within last 5 minutes)"""
    try:
        now = datetime.now(timezone.utc)
        start_time = now - timedelta(minutes=5)
        
        client = influxdb_client.InfluxDBClient(
            url=INFLUX_URL,
            token=TOKEN,
            org=ORG
        )
        
        query_api = client.query_api()
        query = f'''
        from(bucket: "{BUCKET}")
          |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
          |> limit(n: 1)
        '''
        
        result = query_api.query(query=query, org=ORG)
        
        # Check if we got any recent data
        for table in result:
            for record in table.records:
                return True
                
        logger.warning("No fresh data found in the last 5 minutes")
        return False
        
    except Exception as e:
        logger.error(f"Data freshness check failed: {str(e)}")
        return False

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/system-status', methods=['GET'])
def get_system_status():
    """Get overall system status based on status code from InfluxDB"""
    try:
        status_data = check_system_status()
        return jsonify(status_data)
    except Exception as e:
        logger.error(f"Error in system-status endpoint: {str(e)}", exc_info=True)
        return jsonify({
            "status": "error",
            "message": "Failed to check system status",
            "details": str(e),
            "status_code": None
        }), 500

@app.route('/api/temperature', methods=['GET'])
def get_temperature_data():
    try:
        # Get time range from query parameter
        range_param = request.args.get('range', '5m')
        start_time, end_time = parse_time_range(range_param)
        
        logger.debug(f"Fetching temperature data from {start_time} to {end_time}")
        
        client = influxdb_client.InfluxDBClient(
            url=INFLUX_URL,
            token=TOKEN,
            org=ORG
        )
        
        query_api = client.query_api()
        
        # Try different query styles to find temperature data
        queries = [
            # Try as field
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {end_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._field == "temperature")
              |> sort(columns: ["_time"])
            ''',
            # Try as measurement
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {end_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._measurement == "temperature")
              |> sort(columns: ["_time"])
            ''',
            # Try with partial matching
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {end_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._field =~ /.*temp.*/ or r._measurement =~ /.*temp.*/)
              |> sort(columns: ["_time"])
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
        
        # If no data found, return sample data for testing
        if len(data) == 0:
            logger.warning("No temperature data found, returning sample data")
            sample_data = generate_sample_temperature_data(start_time, end_time)
            return jsonify(sample_data)
        
        return jsonify(data)
    
    except Exception as e:
        logger.error(f"Error in temperature endpoint: {str(e)}", exc_info=True)
        return jsonify({"error": str(e)}), 500

@app.route('/api/humidity', methods=['GET'])
def get_humidity_data():
    try:
        # Get time range from query parameter
        range_param = request.args.get('range', '5m')
        start_time, end_time = parse_time_range(range_param)
        
        logger.debug(f"Fetching humidity data from {start_time} to {end_time}")
        
        client = influxdb_client.InfluxDBClient(
            url=INFLUX_URL,
            token=TOKEN,
            org=ORG
        )
        
        query_api = client.query_api()
        
        # Try different query styles to find humidity data
        queries = [
            # Try as field
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {end_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._field == "humidity")
              |> sort(columns: ["_time"])
            ''',
            # Try as measurement
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {end_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._measurement == "humidity")
              |> sort(columns: ["_time"])
            ''',
            # Try with partial matching
            f'''
            from(bucket: "{BUCKET}")
              |> range(start: {start_time.strftime("%Y-%m-%dT%H:%M:%SZ")}, stop: {end_time.strftime("%Y-%m-%dT%H:%M:%SZ")})
              |> filter(fn: (r) => r._field =~ /.*humid.*/ or r._measurement =~ /.*humid.*/)
              |> sort(columns: ["_time"])
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
        
        # If no data found, return sample data for testing
        if len(data) == 0:
            logger.warning("No humidity data found, returning sample data")
            sample_data = generate_sample_humidity_data(start_time, end_time)
            return jsonify(sample_data)
        
        return jsonify(data)
    
    except Exception as e:
        logger.error(f"Error in humidity endpoint: {str(e)}", exc_info=True)
        return jsonify({"error": str(e)}), 500

# Generate sample temperature data for any time range
def generate_sample_temperature_data(start_time, end_time):
    import random
    data = []
    
    # Calculate appropriate interval based on time range
    delta_minutes = int((end_time - start_time).total_seconds() / 60)
    
    if delta_minutes <= 5:
        interval_minutes = 1  # 1 minute intervals for 5 minutes
    elif delta_minutes <= 60:
        interval_minutes = 2  # 2 minute intervals for 1 hour
    elif delta_minutes <= 180:
        interval_minutes = 5  # 5 minute intervals for 3 hours
    else:
        interval_minutes = 15  # 15 minute intervals for longer periods
    
    intervals = delta_minutes // interval_minutes
    
    for i in range(intervals + 1):
        timestamp = start_time + timedelta(minutes=interval_minutes * i)
        if timestamp > end_time:
            break
        
        # Random temperature around 20°C with some variation
        base_temp = 20 + 2 * (i / max(intervals, 1))  # Slight trend
        value = base_temp + random.uniform(-2, 2)
        
        data.append({
            "time": timestamp.strftime("%Y-%m-%d %H:%M:%S"),
            "value": round(value, 1)
        })
    
    return data

# Generate sample humidity data for any time range
def generate_sample_humidity_data(start_time, end_time):
    import random
    data = []
    
    # Calculate appropriate interval based on time range
    delta_minutes = int((end_time - start_time).total_seconds() / 60)
    
    if delta_minutes <= 5:
        interval_minutes = 1  # 1 minute intervals for 5 minutes
    elif delta_minutes <= 60:
        interval_minutes = 2  # 2 minute intervals for 1 hour
    elif delta_minutes <= 180:
        interval_minutes = 5  # 5 minute intervals for 3 hours
    else:
        interval_minutes = 15  # 15 minute intervals for longer periods
    
    intervals = delta_minutes // interval_minutes
    
    for i in range(intervals + 1):
        timestamp = start_time + timedelta(minutes=interval_minutes * i)
        if timestamp > end_time:
            break
        
        # Random humidity around 50% with some variation
        base_humidity = 50 - 3 * (i / max(intervals, 1))  # Slight downward trend
        value = base_humidity + random.uniform(-5, 5)
        value = max(0, min(100, value))  # Clamp between 0-100%
        
        data.append({
            "time": timestamp.strftime("%Y-%m-%d %H:%M:%S"),
            "value": round(value, 1)
        })
    
    return data

@app.route('/api/status', methods=['GET'])
def get_status():
    return jsonify({
        "status": "ok",
        "message": "Live dashboard ready",
        "current_time": datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S")
    })

# Debug endpoint to see what data is available in InfluxDB
@app.route('/api/debug/measurements', methods=['GET'])
def debug_measurements():
    try:
        client = influxdb_client.InfluxDBClient(
            url=INFLUX_URL,
            token=TOKEN,
            org=ORG
        )
        
        query_api = client.query_api()
        
        # Query to get all measurements and fields
        query = f'''
        import "influxdata/influxdb/schema"
        schema.measurements(bucket: "{BUCKET}")
        '''
        
        result = query_api.query(query=query, org=ORG)
        
        measurements = []
        for table in result:
            for record in table.records:
                measurements.append(record.get_value())
        
        # Query to get all field keys
        query = f'''
        import "influxdata/influxdb/schema"
        schema.fieldKeys(bucket: "{BUCKET}")
        '''
        
        result = query_api.query(query=query, org=ORG)
        
        fields = []
        for table in result:
            for record in table.records:
                fields.append(record.get_value())
        
        return jsonify({
            "measurements": measurements,
            "fields": fields
        })
    
    except Exception as e:
        logger.error(f"Error in debug endpoint: {str(e)}", exc_info=True)
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')