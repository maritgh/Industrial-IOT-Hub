## Security Rapport

### Security Measures Implemented

| Area                    | Description |
|-------------------------|-------------|
| **MQTT Broker Security** | The MQTT broker is protected with a username, password, and access control list (ACL). Certificates are prepared but not yet used by the Arduino Portenta Max H7; however, the framework to enable TLS is already in place. |
| **API Authentication (Flask)** | The Flask-based custom API uses API tokens to authenticate with InfluxDB. All database access (read/write) is managed server-side to prevent exposing secrets to the browser. |
| **Frontend Authentication** | One of the dashboards requires user login via username and password. The password is stored securely in an ACL file on the server, where it is validated. However, for testing purposes, the login is currently bypassed in the frontend. |
| **Secrets Management** | API tokens and sensitive credentials are stored securely on the server, never exposed in the frontend or hardcoded in JavaScript. |
| **Docker Containerization** | All services (dashboard, broker, database, data collector) are isolated using Docker containers, reducing the attack surface and improving compartmentalization. |
| **Dependency Management** | Python dependencies are pinned using `requirements.txt`, and service isolation through Docker reduces third-party risks. |
| **Nginx Reverse Proxy** | NGINX is used as a reverse proxy, enabling static file serving, port routing, and optionally HTTPS via SSL certificates. |

---

### Vulnerabilities

| Area              | Risk                                  | Recommendation                                         |
|-------------------|---------------------------------------|--------------------------------------------------------|
| **Wi-Fi Credentials** | SSID and password are hardcoded in Arduino `.ino` files | Use `Secrets.h` to load credentials securely at runtime |
| **MQTT TLS**        | Certificates are available but not fully implemented | Enable TLS                                              |
| **API Rate Limiting** | Vulnerable to abuse                     | Add rate limiting                                       |
| **Input Validation**  | Bad data gets displayed in graphs      | Implement validation of data from sensors and API requests |
