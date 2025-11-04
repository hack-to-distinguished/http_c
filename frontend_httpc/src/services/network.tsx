import { useEffect, useState, useRef } from "react";

export function useWebSocket(serverUrl: string) {
  const [connectionStatus, setConnectionStatus] = useState("Disconnected");
  const socket = useRef<WebSocket | null>(null);
  const reconnectTimeout = useRef<NodeJS.Timeout | null>(null);
  const shouldReconnect = useRef(true); // Track if we should reconnect

  useEffect(() => {
    shouldReconnect.current = true; // Enable reconnection when effect runs

    const connect = () => {
      if (socket.current?.readyState === WebSocket.OPEN || 
          socket.current?.readyState === WebSocket.CONNECTING) {
        console.log("WebSocket already exists, skipping connection");
        return;
      }

      console.log("Attempting to connect to WebSocket...");
      setConnectionStatus("Connecting...");
      
      const ws = new WebSocket(serverUrl);
      socket.current = ws;

      ws.onopen = () => {
        console.log("WebSocket connection established!");
        setConnectionStatus("Connected");
      };

      // ws.onclose = (event) => {
      //   console.log("WebSocket connection closed.", event.code, event.reason);
      //   setConnectionStatus("Disconnected");
      //   socket.current = null;
      //
      //   if (shouldReconnect.current) {
      //     console.log("Scheduling reconnection in 2 seconds...");
      //     setConnectionStatus("Reconnecting...");
      //     reconnectTimeout.current = setTimeout(() => {
      //       connect();
      //     }, 2000);
      //   }
      // };

      ws.onerror = (error) => {
        console.error("WebSocket error: ", error);
        setConnectionStatus("Error");
      };
    };

    connect();

    return () => {
      console.log("Cleanup: Closing WebSocket connection.");
      shouldReconnect.current = false;
      
      if (reconnectTimeout.current) {
        clearTimeout(reconnectTimeout.current);
      }
      
      if (socket.current) {
        if (socket.current.readyState === WebSocket.OPEN || 
            socket.current.readyState === WebSocket.CONNECTING) {
          socket.current.close(1000, "Component unmounting");
        }
        socket.current = null;
      }
    };
  }, [serverUrl]);

  return { socket, connectionStatus };
}
