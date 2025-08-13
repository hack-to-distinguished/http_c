# Project Overview

The aim of this project is to develop proficiency in C socket and network programming. By mastering these skills, we can integrate this project with our other project, [`Tank Squared`](https://github.com/ChristianHrs/tank_squared), enabling real-time in-game communication through chat, as well as supporting custom lobbies that players can host and join.

The next major step is to develop a **database management system from scratch** to track player statistics and manage messaging. This system will also be integrated with the other components, resulting in a fully unified implementation within **Tank Squared**.

**COMPONENTS:**
- Messaging Service
- Web Server
- Database Management System

---

## Table of Contents
- [Simple HTTP_C Web Server](#simple-http_c-web-server)
  - [State Machine Diagram](#state-machine-diagram)
- [Benchmarking Results](#benchmarking-results)
  - [Test Parameters](#test-parameters)
  - [Results](#results)
  - [Observations](#observations)
- [Messaging System](#messaging-system)
- [Project Structure](#project-structure)
  - [Phase 1: TCP Echo Server](#phase-1-tcp-echo-server)
  - [Phase 2: HTTP Server](#phase-2-http-server)
  - [Phase 3: Messaging](#phase-3-messaging)
  - [Phase 4: Integration & Expansion](#phase-4-integration--expansion)
- [How to Compile and Run the HTTP Web Server](#how-to-compile-and-run-the-http-web-server)

---

## Simple HTTP_C Web Server  

A custom-built HTTP/1.1 web server written from scratch in C, designed for **high performance and scalability**.  

**Key Features:**
- Thread pool implementation for efficiently handling multiple simultaneous client connections.
- Stateful HTTP request parsing using a **state machine** for precise and robust request handling.

### State Machine Diagram

<img width="688" height="667" alt="HTTP Request State Machine" src="https://github.com/user-attachments/assets/98b77725-f7ef-4b65-8e52-3d474b0f31fb" />

---

## Benchmarking Results

We evaluated the server using [`wrk`](https://github.com/wg/wrk), a modern HTTP benchmarking tool. All tests were run locally on `127.0.0.1:8080` over 30-second durations.  

### Test Parameters
- **Endpoint tested:** `/static/index.html`  
- **HTTP method:** GET  
- **Duration:** 30 seconds per test  
- **Threads & Connections:** varied to simulate different levels of concurrency

**`/static/index.html`**:
<img width="1625" height="945" alt="image" src="https://github.com/user-attachments/assets/7bcddd8d-2e68-469c-99a4-62340721a2b2" />

### Results

| Threads | Connections | Duration | Requests/sec | Avg Latency | Stdev Latency | Max Latency | Requests Completed | Transfer/sec | Notes |
|--------|------------|---------|-------------|------------|---------------|------------|-----------------|--------------|-------|
| 4      | 100        | 30s     | 34,256      | 2.25ms     | 1.00ms        | 16.43ms    | 1,028,378       | 72.17MB      | Baseline, low concurrency |
| 8      | 1000       | 30s     | 31,521      | 8.91ms     | 42.14ms       | 834.18ms   | 948,533         | 66.40MB      | High concurrency, higher latency and variance |

### Observations
- Moderate concurrency (4 threads, 100 connections) achieves **high throughput (~34k req/s)** with low average latency (~2.25ms).  
- Under very high concurrency (8 threads, 1000 connections), throughput remains strong, but **latency and variability increase significantly**, highlighting limits under extreme load.  
- Minimal socket errors indicate **stable connection handling** even under stress.  

These results demonstrate that the server is **efficient at handling static file requests**, with performance naturally degrading under extreme concurrency.

---

## Messaging System  

A real-time messaging platform leveraging the **WebSocket protocol** for persistent, bidirectional communication.  

**Key Features:**
- TypeScript-based frontend for seamless message sending and receiving.
- Supports group messaging through the server.

---

## Project Structure

### Phase 1: TCP Echo Server
- [x] Create a socket
- [x] Bind to a port
- [x] Listen and accept connections
- [x] Receive data and send it back (echo)

### Phase 2: HTTP Server
- [x] Listen on TCP port
- [x] Apply HTTP protocol to incoming and outgoing data (Ongoing - NJ)
- [x] Large data streaming

### Phase 3: Messaging
- [x] Enable users to send messages to the server
- [ ] Create end-to-end messaging without using the server as a middleman
- [x] Group chat messaging via server redirection
- [ ] Ability to select which client to message
- [ ] GUI for messaging (Ongoing - Christian)
- [ ] Host group chat online with a 6-hour timeout between messages

### Phase 4: Integration & Expansion
- [ ] Use messaging system in **Tank Squared** (other project)
- [ ] Self-host the messaging software
- [ ] Implement blockchain-based decentralized messaging
- [ ] Integrate packet streaming into **Tank Squared** for multiplayer

---

## How to Compile and Run the HTTP Web Server

```bash
# Compile
make all

# Run on default port 8080
./threadpoolserver
