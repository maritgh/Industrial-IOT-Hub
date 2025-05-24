from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import paho.mqtt.client as mqtt
import json

INFLUX_URL = "http://localhost:8086"
TOKEN = "QNU9pj1aTm-fipRW9ZkU5eYvfdAOfVC7pwhX5jdN-lTsx6ZluEIyQyn38oSgRXdG2SSGuVxwnWxPCRFC5wxNvg=="
ORG = "stedin"
BUCKET = "data"

client = InfluxDBClient(url=INFLUX_URL, token=TOKEN, org=ORG)
write_api = client.write_api(write_options=SYNCHRONOUS)

def on_message(mqtt_client, userdata, msg):
    try:
        data = json.loads(msg.payload)
        point = Point("environment") \
            .field("temperature", data["temp"]) \
            .field("humidity", data["hum"])
        write_api.write(bucket=BUCKET, org=ORG, record=point)
        print(f"Stored: {data}")
    except Exception as e:
        print(f"Error: {e}")

mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message
mqtt_client.connect("192.168.178.165", 1883, 60)
mqtt_client.subscribe("sensor/data")
mqtt_client.loop_forever()
