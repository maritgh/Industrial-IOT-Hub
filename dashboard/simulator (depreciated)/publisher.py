# The goal of this code is to emulate the arduino, this is for development purposes when you dont have the Arduino on hand
# it does this by sending dummy data to the MQTT broker
# This is the usual data flow
# Arduino -> MQTT Broker -> Collector.py -> InfluxDB -> frontend 
# The emulator changes it to 
# publisher.py -> MQTT Broker -> Collector.py -> InfluxDB -> frontend 
# simply run this python script and launch the Docker containers by in terminal heading to Industrial-IOT-Hub\dashboard\ 
# and running "docker-compose build" and "docker-compose up -d"

import paho.mqtt.client as mqtt
import time
import random
import json

client = mqtt.Client()
client.connect("192.168.178.165", 1883, 60) #change this to preferably your ipv4 adress or the one of the docker container



# every few second its sends random dummy data to the MQTT broker
while True:
    data = {
        "temp": round(random.uniform(20.0, 21.0), 2),   
        "hum": round(random.uniform(40.0, 41.0), 2),
        "status": round(random.uniform(0, 2), 2),
        "pressure": 1000.11
    }
    payload = json.dumps(data)
    client.publish("sensor/data", payload)
    print(f"Published: {payload}")
    time.sleep(5)

