import { useState, useEffect, useRef } from "react";
import { useWebSocket } from "../services/network.tsx";
import NetworkStatus from "../components/networkStatus.tsx";
import MessageBox from "../components/messageBox.tsx";

function MessageDisplay() {

  const serverUrl = "ws://127.0.0.1:8080";
  const { socket, connectionStatus } = useWebSocket(serverUrl);

  return (
    <>
      <div>
        <NetworkStatus socket={socket} connectionStatus={connectionStatus} />
        <MessageBox socket={socket} connectionStatus={connectionStatus} />
      </div>
    </>
  )
}

export default MessageDisplay;
