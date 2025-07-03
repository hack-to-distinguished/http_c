import { useState, useEffect, useRef } from "react";
import "./components.css";

function MessageDisplay() {
  // --- WebSocket Configuration ---
  const serverUrl = "ws://127.0.0.0:8080"; // Note: ws:// for WebSockets

  // --- State Management ---
  // Store the socket instance in a ref to avoid re-creating it on re-renders
  const socket = useRef(null);
  // Track the current connection status
  const [connectionStatus, setConnectionStatus] = useState("Disconnected");
  // Store incoming messages in an array
  const [messages, setMessages] = useState([]);
  // Store the message we are currently typing
  const [currentMessage, setCurrentMessage] = useState("");

  useEffect(() => {
    // This effect runs once when the component mounts
    console.log("Attempting to connect to WebSocket...");
    setConnectionStatus("Connecting...");

    // Create a new WebSocket instance. This is what starts the connection.
    socket.current = new WebSocket(serverUrl);

    // --- Define WebSocket Event Listeners ---

    // 1. Fired when the connection is successfully established
    socket.current.onopen = () => {
      console.log("WebSocket connection established!");
      setConnectionStatus("Connected");
    };

    // 2. Fired when a message is received from the server
    socket.current.onmessage = (event) => {
      // event.data contains the message payload from the server
      const receivedMessage = event.data;
      console.log("Message from server: ", receivedMessage);

      // Add the new message to our list of messages
      // Use a functional update to get the previous state correctly
      setMessages(prevMessages => [...prevMessages, receivedMessage]);
    };

    // 3. Fired when the connection is closed
    socket.current.onclose = () => {
      console.log("WebSocket connection closed.");
      setConnectionStatus("Disconnected");
    };

    // 4. Fired when an error occurs
    socket.current.onerror = (error) => {
      console.error("WebSocket error: ", error);
      setConnectionStatus("Error");
    };

    // --- Cleanup Function ---
    // This function is returned by the effect and runs when the component unmounts.
    // It's crucial for closing the connection and preventing memory leaks.
    return () => {
      if (socket.current) {
        console.log("Closing WebSocket connection.");
        socket.current.close();
      }
    };
  }, []); // The empty dependency array [] means this effect runs only once on mount.

  // --- Handler for Sending a Message ---
  const handleSendMessage = (e) => {
    e.preventDefault(); // Prevent form from reloading the page
    // if (currentMessage && socket.current?.readyState === WebSocket.OPEN) {
    if (currentMessage) {
      console.log(`Sending message: ${currentMessage}`);
      socket.current.send(currentMessage); // Send the message
      setCurrentMessage(""); // Clear the input box
    } else {
      console.log("Cannot send message. WebSocket is not open or message is empty.");
    }
  };

  return (
    <div className="chat-container">
      <h1>Simple WebSocket Client</h1>
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
        {/* disabled={connectionStatus !== "Connected"} */}
        <button type="submit">
          Send Message
        </button>
      </form>
    </div>
  );
}

export default MessageDisplay;




//////////////// PREV /////////////////////////
// import React, { useState } from "react";
// import "./components.css"
//
// function MessageBox() {
//   // Somehow call the C function, API or whatever from here
//   const [msg, setMsg] = useState(""); 
//
//   function handleOnChange(event) {
//     setMsg(event.target.value);
//   }
//
//   async function handleFormSubmit(event) {
//     event.preventDefault();
//     console.log("Message sent:", msg);
//
//     await fetch("http://127.0.0.0:8080", {
//       method: "POST",
//       headers: { "Content-Type": "text/plain" },
//       body: msg
//     })
//       .then(res => res.text())
//       .then(console.log);
//     setMsg("");
//   }
//
//   return (
//     <>
//       <form onSubmit={handleFormSubmit}>
//         <div className="div_input_message">
//           <label className="label_input_message">What are you thinking?</label>
//           <input type="text" id="messageInput" value={msg} onChange={handleOnChange} className="input_message_box"/>
//         </div>
//       </form>
//     </>
//   )
// }
//
// export default MessageBox
