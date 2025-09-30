import { useState, useEffect } from "react";
import "./messageFeed.css";

interface MessageBoxProps {
  socket: WebSocket;
  connectionStatus: string;
}

const MessageFeed = ({ socket, connectionStatus }: MessageBoxProps) => {
  const [messages, setMessages] = useState<string[]>([]);

  useEffect(() => {
    if (!socket.current) return;

    const handleMessage = (event: MessageEvent) => {
      const receivedMessage = event.data;
      console.log("Event", socket.current);
      console.log("Message from server:", receivedMessage);
      setMessages((prevMessages) => [...prevMessages, receivedMessage]);
    };

    socket.current.onmessage = handleMessage;

    return () => {
      if (socket.current) {
        socket.current.onmessage = null;
      }
    };
  }, [socket.current]);

  return (
    <div className="messages-display">
      <div className="message-header">HTTP_C Chat</div>
      <ul className="messages-list">
        {messages.map((msg, index) => (
          <li key={index}>
            <span style={{ color: "#003366", fontWeight: "bold" }}>
              {new Date().toLocaleTimeString()} - Message {index + 1}:
            </span>{" "}
            {msg}
          </li>
        ))}
        {messages.length === 0 && (
          <li className="empty-message">No messages yet.</li>
        )}
      </ul>
    </div>
  );
};

export default MessageFeed;
