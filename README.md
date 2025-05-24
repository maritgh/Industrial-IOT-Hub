# Industrial-IOT-Hub
# Sensor Data Collector (MQTT to InfluxDB)

This project collects simulated sensor data via MQTT and stores it in an InfluxDB 2.x database using a Python backend.

---

## 📡 Network Overview
[Python Simulator] ---> MQTT ---> [Python Collector] ---> InfluxDB

*Note: Arduino connection is excluded for simulation purposes.*

---

## Setup Guide

Run you container

After running the container, you can access the InfluxDB UI at `http://localhost:8086`.

You will need to create an initial user, organization, and a bucket. For this tutorial, let's assume you create a bucket named `data`. These have already been setup

### 1\. Configure Bucket and MQTT Consumer in InfluxDB

To configure the MQTT consumer, follow these steps within the InfluxDB UI:

1.  Navigate to the bucket you created (e.g., `data`).
2.  Click on the `Add Data` button.
3.  Select `Telegraf Configuration`.
4.  Choose `MQTT Consumer`.
5.  Configure the MQTT Consumer connection settings. The default MQTT broker address is usually `localhost:1883`.
6.  Click `Continue Configuring`.
7.  In the `View Configuration` step, you will be prompted to paste a configuration file. The location of a sample configuration file in this project is:

    ```bash
    backend/telegraf configuration/
    ```
8.  Save the Telegraf configuration.
9.  After saving, copy the generated API token. You will need this token in the Python backend configuration.

### 3\. Setup Python Backend

First, install the necessary Python packages:

```bash
pip install influxdb-client paho-mqtt
pip3 install flask-cors

```

Next, you need to edit the `backend/collector.py` file to include your InfluxDB connection details. Open the file and update the following variables:

```python
INFLUX_URL = "http://localhost:8086"
TOKEN = "YOUR_API_TOKEN_HERE" # Replace with the API token you copied 
ORG = "your-org"             # Replace with your InfluxDB organization name in this case "Stedin"
BUCKET = "data"              # Ensure this matches the bucket you created "For now it is "Data"
```

Once you have configured the backend, you can run the collector script to start listening for MQTT messages and writing data to InfluxDB:

```bash
python backend/collector.py
```

### 4\. Run the Data Simulator (Dummy Data Publisher)

To generate dummy sensor data and publish it over MQTT, run the simulator script:

```bash
python simulation/simulator.py
```

This script will simulate temperature and humidity readings and send them to the MQTT broker, which will then be processed by the Python collector and stored in InfluxDB.
```