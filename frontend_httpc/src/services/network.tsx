import { useEffect, useState, useRef } from "react";

export function useWebSocket(serverUrl: string) {

  const [connectionStatus, setConnectionStatus] = useState("Disconnected");
  const socket = useRef<WebSocket | null>(null);

  useEffect(() => {
    console.log("Attempting to connect to WebSocket...");
    setConnectionStatus("Connecting...");

    socket.current = new WebSocket(serverUrl);

    socket.current.onopen = () => {
      console.log("WebSocket connection established!");
      setConnectionStatus("Connected");
    };

    socket.current.onclose = () => {
      console.log("WebSocket connection closed.");
      setConnectionStatus("Disconnected");
    };

    socket.current.onerror = (error) => {
      console.error("WebSocket error: ", error);
      setConnectionStatus("Error");
    };

    return () => {
      if (socket.current) {
        console.log("Closing WebSocket connection.");
        socket.current.close();
      }
    };
  }, [serverUrl]);

  return { socket, connectionStatus };
}
