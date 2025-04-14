# Chat Application

This is a real-time chat application using **MQTT, Docker, Nginx, and WebSockets**. It features **secure authentication** via Mosquitto password files and access control lists (ACLs). The frontend is served via Nginx, however websockets is as of yet not properly implemented, but the foundations for it have been laid down

## Features
- **User Authentication** with username and password
- **Real-time messaging** using MQTT
- **WebSocket support** for browser-based clients
- **Secure communication** via TLS (Nginx SSL termination)
- **Containerized deployment** with Docker

## Architecture
```
Frontend  -->  Nginx (Reverse Proxy, SSL)  -->  Mosquitto (MQTT Broker)
                                                |
                                                |
                                            Database 
```

### Network Flow
Look at the network diagram

## Setup Instructions

### Prerequisites
- Docker & Docker Compose installed
- OpenSSL (for generating SSL certificates)
- Mosquitto has been installed for generating the password files and implementing it into docker

Run Docker Compose:
```sh
docker-compose build
```
```sh
docker-compose up -d
```

This starts the Nginx server, Mosquitto broker, and the frontend application.

### Configuration
#### **Mosquitto Authentication**
- Passwords and users are stored in `mosquitto/password_file`.
- Passwords for the frotend is stored in the `public/chat.js` file itself under "users"

- For a security in "mosquitto/mosquitto.conf" allow_anonymous must be turned off. 

To add a new user:
```sh
mosquitto_passwd -b mosquitto/password_file newuser newpassword
```
then type in your new password twice. The user AND password in the chat.js and in the password_file has to match otherwise it wont work. 
For debugging purposes or adding a dummy account set "allow_anonymous" to true

#### **Nginx Configuration**
Modify `nginx/nginx.conf` to customize routing and security settings.

#### **SSL Setup (Optional)**
To enable HTTPS, place certificates in `ssl/` and update `nginx.conf`:
```sh
ssl_certificate /etc/nginx/ssl/cert.crt;
ssl_certificate_key /etc/nginx/ssl/cert.key;
```

## Usage
1. Open `https://localhost` in a browser on multible tabs.
2. Log in using a valid MQTT username/password, in each tab on a different account
3. Start sending messages in real time!




## License
Made by Derk Ottersberg
