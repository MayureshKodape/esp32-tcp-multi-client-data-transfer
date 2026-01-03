# ESP32 TCP Multi-Client Data Transfer

## Overview
This project demonstrates how an ESP32 can act as a TCP server
and handle multiple client connections simultaneously.

It is designed to understand:
- TCP socket programming on ESP32
- Handling multiple clients reliably
- Real-time data transfer in IoT applications

## Why TCP?
TCP is used instead of UDP to ensure:
- Reliable data delivery
- Ordered packets
- Connection-based communication

This is useful for industrial and automation systems
where data integrity is critical.

## Architecture
- ESP32 acts as a TCP Server
- Multiple clients connect via IP + Port
- ESP32 listens, accepts, and processes client data
- Data is received and optionally echoed / processed

(Client handling is implemented using tasks / loops)

## Hardware Used
- ESP32 Dev Module
- Wi-Fi Network (same LAN)
- TCP clients (PC / Mobile / Python script)

## Software Stack
- ESP-IDF / Arduino (mention which one clearly)
- Wi-Fi TCP/IP stack
- Socket APIs

## How to Run
1. Flash the code to ESP32
2. Connect ESP32 to Wi-Fi
3. Note the ESP32 IP address
4. Connect multiple clients using:
   - PC terminal
   - Python socket script
5. Send data and observe responses

## Use Cases
- IoT gateway
- Multi-device control system
- Data aggregation node
- Industrial communication prototype

## Project Status
✔ Basic multi-client handling implemented  
🚧 Improvements planned:
- Client timeout handling
- Better error handling
- Dynamic client limit

## Author
Mayuresh Kodape
