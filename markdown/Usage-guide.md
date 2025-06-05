## Setup Instructions

To get started with the Industrial IoT Hub project, follow the steps below to set up both the IoT hardware and the dashboard interface.

### 1. Setting Up the IoT Hub (Arduino + ESP32s)

The IoT Hub is responsible for collecting environmental sensor data (e.g., temperature and humidity) from multiple ESP32 devices and forwarding it to a central server.

To configure and deploy the IoT Hub system:

For detailed setup steps, wiring diagrams, and code configuration, **please refer to the [Arduino Usage Guide](ArduinoUsage.md)**.

> ⚠️ **Note:** Both the IoT Hub (ESP32s) and the dashboard machine must be connected to the same Wi-Fi network. This ensures that the devices can communicate with each other locally without requiring external routing. In many cases the IP adresses, and the network name and password will need to be changed in this project code. Eeach guide will cover this

---

### 2. Setting Up the Dashboard

The dashboard is used to visualize the incoming data from all IoT Hubs in real-time. It allows users to monitor sensor values across different locations via a unified interface.

To set up the dashboard:

* Clone the repository and navigate to the dashboard folder.
* Follow the installation instructions to run the frontend.
* Ensure that the dashboard is connected to the backend server or database receiving data from the IoT Hub.
* Launch the dashboard in your browser to start monitoring live data.

For full installation and configuration instructions, **please refer to the [Dashboard Usage Guide](DashboardUsage.md)**.

---

Once both components are set up, you will have a working Industrial IoT system capable of collecting, processing, and visualizing sensor data from transformer stations.
