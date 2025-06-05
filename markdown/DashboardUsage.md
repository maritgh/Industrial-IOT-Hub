# Dashboard Usage Guide

---

This setup guide explains how to launch the current project, followed by instructions to replicate it yourself.

---

## Table of Contents
- [Launching the Dashboard](#dashboard-launch)
  - [Cloning the git repository](#cloning-the-git-repository)
  - [Changing the IP adresses to match your setup](#changing-the-ip-adresses-to-match-your-setup)
  - [Installing Docker](#installing-docker)
  - [Lauching the project](#launching-docker)
    - [The dashboard](#the-dashboard)
* [Deep dive in the Dashboard Setup](#dashboard-setup)

---

## Dashboard Launch
### Cloning the git repository
1. Head to the git repository
2. Click the green ``` <> code ``` and copy the link
3. Create a new project in your preffered code editor
4. In the termial type ```git clone "link"```

### Changing the IP adresses to match your setup
First things first. It is crucial to also change the IP adress on every device
   * The main host computer that is running the docker container
   * The Arduino Portenta Max H7 (the main IOT-hub)
   * The Arduino ESP32's (The devices with attached sensors)

1. Open [collector.py](../dashboard/backend/collector.py) and change the IP on the line:

   ```python
   ip_adress = "xxx.xxx.xxx.xxx"
   ```

   to your own IPv4 address, which can be found by typing `ipconfig` in the terminal.


### Installing docker
1. Head to ```https://www.docker.com``` 
2. install docker at the download page
3. Follow the install wizard
   
### Launching Docker
1. Make sure docker is running
2. Open the terminal in ```Industrial-IOT-Hub\dashboard\```
3. Run:

   ```bash
   docker-compose build
   docker-compose up -d
   ```

   All containers and the dashboard will be launched.
#### The dashboard
Our project contains two dashboard
   * [Real time dashboard](#real-time-dashboard)
   The real time dashboard displays live data wich is collected by the IOT-hub in graphs
   * [Test dashboard](#test-dashboard)
   This dashboard is an example of what an official stedin website for checking sensor data. Stedin has multible powerstation, hence it should be easy to acces multible different stations, through one dashboard. It also features a login screen

### Real time dashboard
1. To access the dashboard, open the `dashboard-app` container:

   ![Dashboard Access](../images/dashboardip.png)

   This address is where the dashboard is hosted, paste it into your browser

### Test dashboard

   ![image](../images/testdashboard.png)
   The above dashboard primary purpose is to be a clean overview wich can acces all graphs of around the country. It is important to note that this is a demo wich is includes dummy sensor data, rather than live data that is retrieved by the pyhiscal sensors.
   Acces this dashboard by heading to ```localhost:80``` in your browser

---

## Dashboard Setup

### 📡 Network Overview

![Network Flow](../images/networkflow.png)

The network flow consists of multiple parts. This guide walks you through the setup so it can be replicated. Every process in the diagram is launched through Docker containers.

---

### 1. Connect the Hub to the Broker

Please refer to the [Arduino Usage Guide](ArduinoUsage.md).

---

### 2. InfluxDB

1. InfluxDB is the database used. It has its own UI when the Docker container is launched. To access it, open:

   ```
   http://localhost:8086
   ```

2. To log in, head to [docker-compose.yml](../dashboard/docker-compose.yml).

3. In the InfluxDB service, the credentials are shown:

   ```yaml
   - DOCKER_INFLUXDB_INIT_USERNAME=admin
   - DOCKER_INFLUXDB_INIT_PASSWORD=yourpassword
   ```

4. For documentation, visit: `https://docs.influxdata.com`

5. A setup is needed consisting of:

   * An initial user
   * An organization
   * A new bucket ("data" is already set up)

---

### 3. The Data Collector

[collector.py](../dashboard/backend/collector.py) checks whether new data has been posted on the `sensor/data` topic in the MQTT broker. Once new data is found, it stores it in the database.

#### API Token

1. In InfluxDB, press "Load Data" and navigate to the `API` submenu.
2. Here you can generate an API token that controls read and write access to your database.

This API token is used in [collector.py](../dashboard/backend/collector.py) to enable it to read data from the MQTT broker and store it in InfluxDB.

---

### 4. Dashboard and Custom API

For storing data in the database and displaying it, a custom API is used. This API also scrapes data from the database and serves it to the frontend for visualization.

Both the Custom API and the Frontend reside in the RealTimeDashboard folder. The folder structure is organized so that all dashboard logic (backend and frontend) is encapsulated in one place for easier maintenance and deployment.

Both the [Custom API](../dashboard/RealTimeDashboard/app.py) and the [Frontend](../dashboard/RealTimeDashboard/templates/index.html) reside in the `RealTimeDashboard` folder.

In this script the token wich manages read and write acces to the InfluxDB database is stored

The API acts as a secure intermediary between the backend (InfluxDB) and the frontend. Since direct access to InfluxDB requires sensitive credentials (such as an API token), which must be kept secure, the API performs all database queries on behalf of the frontend. This ensures the frontend never exposes any critical secrets or credentials to the browser.

Additionally, the API layer allows you to preprocess, aggregate, and format data before sending it to the frontend. This separation of concerns improves scalability, security, and maintainability of the overall system.

---
### 5. Docker Compose Overview

This [docker-compose.yml](../dashboard/docker-compose.yml) file defines the services required to run a secure and integrated Industrial IoT dashboard system. Below is a brief explanation of each service and its role in the architecture.



##### Services

### 1. **web (Nginx)**

* **Purpose:** Acts as a reverse proxy and serves static files.
* **Ports:** Exposes HTTP (80) and HTTPS (443).
* **Volumes:**

  * SSL certificates.
  * Nginx config.
  * Static web files.
*  **Used by:** The DemoDashboard

### 2. **mosquitto (MQTT Broker)**

* **Purpose:** Handles real-time message passing between the IOT-hub and services.
* **Ports:** 1883 for standard MQTT, 1884 for alternative use.
* **Security:** Includes configuration, password, and ACL files. SSL support enabled.
*  **Used by:** Both the RealTimeDashboard and the DemoDashboard



### 3. **dashboard\_app (Flask Dashboard)**

* **Purpose:** Hosts the real-time web dashboard for data visualization.
* **Runs:** Python Flask app (`app.py`).
* **Port:** 5000 (web interface).
* **Dependencies:** Installed at container startup.

### 4. **data\_collector (MQTT to InfluxDB Bridge)**

* **Purpose:** Listens to MQTT topics and stores data in InfluxDB.
* **Dependencies:** Requires both Mosquitto and InfluxDB to be up first.

### 5. **influxdb**

* **Purpose:** Time-series database storing IoT sensor data.
* **Port:** 8086 (web UI & API access).
* **Initialization:** Automatically creates a user, org, bucket, and admin token.

---

## Network

All containers communicate via a shared Docker bridge network named `app-network`.

---

## Running the Stack

To launch all services:

```bash
docker-compose build
docker-compose up -d
```

This setup ensures a modular, scalable, and secure IoT infrastructure that is easy to deploy and maintain.
