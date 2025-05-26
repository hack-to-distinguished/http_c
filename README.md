# 🌐 C HTTP Server

A bare-bones HTTP server written in C from scratch using low-level socket programming. This project walks through core networking concepts like TCP, echo servers, request parsing, and HTTP response construction.

---

## 🧱 Project Structure

### ✅ Phase 1: TCP Echo Server

-[x] Create a socket
-[x] Bind to a port
-[x] Listen and accept connections
-[x] Receive data and send it back (echo)

---

### ✅ Phase 2: HTTP Server

-[x] Listen on TCP port
-[ ] Apply HTTP protocol to incoming and outgoing data (Ongoing - NJ)

---

### Phase 3: Messaging

-[x] Enable users to send messages to the server
-[ ] Create end to end messaging without using the server as a middle man
-[ ] Create group chat style messaging by messaging the server and it redirect the message to all clients (Ongoing - Christian)

---


### Phase 4: Integration & Expansion

- [ ] Use the messaging system created here to enable chat in Tank Square (other project)
- [ ] Self host the messaging software
- [ ] Use blockchain technology to create decentralized messaging system

---


### ⚙️ How to Compile and Run

```bash
# Compile
gcc Server.c -o http_server

# Run (on default port 8080)
./http_server
```
