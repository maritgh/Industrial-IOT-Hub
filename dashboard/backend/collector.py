from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import paho.mqtt.client as mqtt
import json
import time

INFLUX_URL = "http://host.docker.internal:8086"
TOKEN = "QNU9pj1aTm-fipRW9ZkU5eYvfdAOfVC7pwhX5jdN-lTsx6ZluEIyQyn38oSgRXdG2SSGuVxwnWxPCRFC5wxNvg=="
ORG = "stedin"
BUCKET = "data"
ip_adress = "192.168.178.165" #verander dit naar je ipv4 adress

client = InfluxDBClient(url=INFLUX_URL, token=TOKEN, org=ORG)
write_api = client.write_api(write_options=SYNCHRONOUS)

# Retry InfluxDB connection
influx_connected = False
while not influx_connected:
    try:
        client = InfluxDBClient(url=INFLUX_URL, token=TOKEN, org=ORG)
        client.ping()  # Confirm it's alive
        write_api = client.write_api(write_options=SYNCHRONOUS)
        influx_connected = True
        print("Connected to InfluxDB.")
    except Exception as e:
        print(f"Waiting for InfluxDB: {e}")
        time.sleep(3)

def on_message(mqtt_client, userdata, msg):
    try:
        data = json.loads(msg.payload)
        point = Point("environment") \
            .field("temperature", data["temp"]) \
            .field("humidity", data["hum"]) \
            .field("status_code", data["status"])
        write_api.write(bucket=BUCKET, org=ORG, record=point)
        print(f"Stored: {data}")
    except Exception as e:
        print(f"Error: {e}")

mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message
mqtt_client.connect(ip_adress, 1883, 60)
mqtt_client.subscribe("sensor/data")
mqtt_client.loop_forever()