import { useState, useEffect, useRef } from "react";
import "./messageBox.css";

function MessageBox() {
  const serverUrl = "ws://127.0.0.1:8080";

  // Store the socket instance in a ref to avoid re-creating it on re-renders
  const socket = useRef(null);
  const [connectionStatus, setConnectionStatus] = useState("Disconnected");
  const [messages, setMessages] = useState([]);
  const [currentMessage, setCurrentMessage] = useState("");

  useEffect(() => {
    console.log("Attempting to connect to WebSocket...");
    setConnectionStatus("Connecting...");

    socket.current = new WebSocket(serverUrl);

    socket.current.onopen = () => {
      console.log("WebSocket connection established!");
      setConnectionStatus("Connected");
    };

    socket.current.onmessage = (event) => {
      const receivedMessage = event.data;
      console.log("Message from server: ", receivedMessage);

      // Add the new message to our list of messages
      // Use a functional update to get the previous state correctly
      setMessages(prevMessages => [...prevMessages, receivedMessage]);
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
  }, []);

  const handleSendMessage = (e) => {
    e.preventDefault();
    if (currentMessage && socket.current?.readyState === WebSocket.OPEN) {
    // if (currentMessage) {
      console.log(`Sending message: ${currentMessage}`);
      socket.current.send(currentMessage);
      setCurrentMessage("");
    } else {
      console.log("Cannot send message. WebSocket is not open or message is empty.");
    }
  };

  return (
    <div className="chat-container">
      <div className="status">
        <strong>Connection Status:</strong> {connectionStatus}
      </div>

      <div className="message-display">
        <h2>Messages from Server:</h2>
        <ul>
          {messages.map((msg, index) => (
            <li key={index}>{msg}</li>
          ))}
          {messages.length === 0 && <li>No messages yet.</li>}
        </ul>
      </div>

      <form onSubmit={handleSendMessage} className="message-form">
        <input
          type="text"
          value={currentMessage}
          onChange={(e) => setCurrentMessage(e.target.value)}
          placeholder="Type a message..."
        />
        <button type="submit" disabled={connectionStatus !== "Connected"}>
          Send Message
        </button>
      </form>
    </div>
  );
}

export default MessageBox;
