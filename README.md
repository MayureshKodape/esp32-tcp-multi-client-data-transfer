# esp32-tcp-multi-client-data-transfer
ESP32 TCP client project that connects to a server, supports multiple clients, and demonstrates reliable data transfer of 100,000 bytes using ESP-IDF.

# 🚀 ESP32 TCP Multi-Client Data Transfer  

This repository contains an **ESP32 TCP client project** built with the **ESP-IDF framework**.  
It demonstrates how to connect an ESP32 device to a **TCP server** over Wi-Fi, exchange data with **multiple clients**, and test **large data transfer (100,000 bytes)**.  

---

## 📖 Table of Contents  

1. [Overview](#overview)  
2. [Features](#features)  
3. [Project Structure](#project-structure)  
4. [Requirements](#requirements)  
5. [How It Works](#how-it-works)  
6. [Setup & Build](#setup--build)  
7. [Running the Server](#running-the-server)  
8. [Monitoring ESP32 Output](#monitoring-esp32-output)  
9. [Example Logs](#example-logs)  
10. [Performance Notes](#performance-notes)  
11. [Future Improvements](#future-improvements)  
12. [License](#license)  

---

## 🔎 Overview  

- The **ESP32 acts as a TCP client**, connecting to a server running on a PC/Raspberry Pi/other system.  
- Multiple ESP32 boards can connect simultaneously (multi-client).  
- Each client requests **100,000 bytes of data** from the server to test reliability and speed.  
- Logs show how many bytes were received, along with timestamps for performance analysis.  

---

## ✨ Features  

- 📡 **Wi-Fi STA mode** with auto-reconnect  
- 🔗 **TCP client** using lwIP sockets  
- 👥 Works with **multiple ESP32 clients** connected to one server  
- 📦 Supports **large payload transfer** (100k bytes demo)  
- ⏱ **Timestamp logging** to measure speed & latency  
- 🔄 Automatic retry if server disconnects  


## 🛠 Requirements  

- ESP32 development board (ESP32, ESP32-S2, ESP32-S3, etc.)  
- [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/) (tested with v5.1.6)  
- Python 3.x (for server script)  
- A PC/Raspberry Pi to run the TCP server  

---

## ⚙️ How It Works  

1. ESP32 boots and connects to your Wi-Fi network.  
2. The ESP32 creates a **TCP socket** and attempts to connect to the server (IP + port).  
3. Once connected, the server sends **100,000 bytes** of test data.  
4. ESP32 receives chunks of data, logs the received size, and keeps a running total.  
5. After the full payload is received, the ESP32 closes the connection and retries.  
6. Multiple ESP32 boards can connect simultaneously to the same server.  

---

## 🚀 Setup & Build  

### 1. Configure Wi-Fi  
Edit the `main/tcp_client.c` file and update:  

```c
#define WIFI_SSID   "YourWiFiSSID"
#define WIFI_PASS   "YourWiFiPassword"
#define SERVER_IP   "192.168.1.100"  // Your server's IP
#define SERVER_PORT 3333

