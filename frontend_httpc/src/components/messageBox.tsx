import { useState, useEffect, useRef } from "react";
import "./messageBox.css";

function MessageBox({socket, connectionStatus}) {

  const [messages, setMessages] = useState([]);
  const [currentMessage, setCurrentMessage] = useState("");

  // useEffect(() => {
  //   socket.current.onmessage = (event) => {
  //     const receivedMessage = event.data;
  //     console.log("Message from server: ", receivedMessage);
  //
  //     // Add the new message to our list of messages
  //     // Use a functional update to get the previous state correctly
  //     setMessages(prevMessages => [...prevMessages, receivedMessage]);
  //   };
  //
  //   return () => {
  //     // if (socket.current) {
  //     //   console.log("Closing WebSocket connection.");
  //     //   socket.current.close();
  //     // }
  //   };
  // }, [socket]);

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
