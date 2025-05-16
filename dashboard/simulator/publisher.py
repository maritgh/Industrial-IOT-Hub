import paho.mqtt.client as mqtt
import time
import random
import json

client = mqtt.Client()
client.connect("localhost", 1883, 60)

while True:
    data = {
        "temp": round(random.uniform(20.0, 25.0), 2),
        "hum": round(random.uniform(30.0, 50.0), 2)
    }
    payload = json.dumps(data)
    client.publish("sensor/data", payload)
    print(f"Published: {payload}")
    time.sleep(5)
