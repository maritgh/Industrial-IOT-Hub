
## Dashboard Usage Guide
---
First in this setup there will be a guide on how to launch the current project, therafter a guide will be made on how you can replicated this yourself.

  - [Dashboard launch](#dashboard-launch)
  - [Dashboard Setup](#dashboard-setup)
---
## Dashboard Launch
1 clone the git repository
2 open in the terminal Industrial-IOT-Hub\dashboard
3 docker-compose build 
4 docker-compose up -d

All the containers and the dashboard is launched


---





## Dashboard Setup



#### 📡 Network Overview

![Alt text](images/networkflow.png)
---


the network flow consist of multible parts. This guide will guide you through the setup so it can be replicated. Every process in this diagram is launched through the docker containers



### 1\. Connect the hub to the Broker
**please refer to the [Arduino Usage Guide](ArduinoUsage.md)**.

### 2\. The data collector
This python script checks wheter new data has been posted on the "sensor/data" topic. Once it find new data it stores it into the Database
[collector.py](dashboard/backend/collector.py)
### 3\. Configure Bucket and MQTT Consumer in InfluxDB


You will need to create an initial user, organization, and a bucket. For this tutorial, let's assume you create a bucket named `data`. These have already been setup
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




```python
INFLUX_URL = "http://localhost:8086"
TOKEN = "YOUR_API_TOKEN_HERE" # Replace with the API token you copied 
ORG = "your-org"             # Replace with your InfluxDB organization name in this case "Stedin"
BUCKET = "data"              # Ensure this matches the bucket you created "For now it is "Data"
```

### 4\. Dashboard and API
Both the [Api](dashboard\RealTimeDashboard\app.py) and the [Frontend](dashboard\RealTimeDashboard\templates\index.html) are in the RealTimeDashboard folder. This api isnt just an API however, it also launches the dashboard itself and hosts it
The Api Scrapes the data from the databases, this is for security reasons since in order to gather data from the database you need a key. Hosting this key itself on the frontend is a security risk, hence the data flows from the api to the frontend and not directly.


