import { useState, useEffect } from "react";
import { recvMessage } from "../services/recv_send.tsx";
import "./messageFeed.css";

interface MessageBoxProps {
  socket: WebSocket;
  connectionStatus: string;
}

const MessageFeed = ({ socket, connectionStatus }: MessageBoxProps) => {
  const [messages, setMessages] = useState<string[]>([]);


  useEffect(() => {
    if (!socket) return;
    console.log("SOCKET:", socket);

    const handleMessage = (event: MessageEvent) => {
      const receivedMessage = event.data;
      console.log("Message from server: ", receivedMessage);
      setMessages(prevMessages => [...prevMessages, receivedMessage]);
    };

    socket.onmessage = handleMessage;

    return () => {
      if (socket) {
        socket.onmessage = null;
      }
    };
  }, [socket]);


  return (
    <div className="messages-display">
      <ul className="messages-list">
        {messages.map((msg, index) => (
          <li key={index}>{msg}</li>
        ))}
        {messages.length === 0 && <li>No messages yet.</li>}
      </ul>
    </div>
  );
}

export default MessageFeed;
