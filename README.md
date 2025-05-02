# 🌐 C HTTP Server

A bare-bones HTTP server written in C from scratch using low-level socket programming. This project walks through core networking concepts like TCP, echo servers, request parsing, and HTTP response construction.

---

## 📦 What This Project Does

- Creates a **TCP server** on a specified port (default `8080`)
- Accepts incoming connections
- Parses **basic HTTP GET requests**
- Sends back **hardcoded HTML content**
- Can be accessed from any device on the same network

---

## 🧱 Project Structure & Learning Milestones

### ✅ Phase 1: TCP Echo Server

-[x] Create a socket
-[x] Bind to a port
-[x] Listen and accept connections
-[x] Receive data and send it back (echo)

> 💡 Used to get familiar with sockets, ports, TCP connection lifecycle

---

### ✅ Phase 2: HTTP Server

-[x] Listen on TCP port
-[x] Read and parse **incoming HTTP requests**

---

### Phase 3: Messaging

-[x] Enable users to send messages to the server
-[ ] Create end to end messaging without using the server as a middle man
-[ ] Create group chat style messaging by messaging the server and it redirect the message to all clients (Ongoing - Christian)

---

### ⚙️ How to Compile and Run

```bash
# Compile
gcc Server.c -o http_server

# Run (on default port 8080)
./http_server
