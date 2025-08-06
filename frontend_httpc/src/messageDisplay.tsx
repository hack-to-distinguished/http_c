import { useState, useEffect } from "react";
import "./components.css";
import axios from "axios";

function MessageDisplay() {

  const serverUrl = "http://127.0.0.1:8080";
  async function receiveMessage() {
    await fetch(serverUrl, {
      method: "GET",
    })
      .then(res => res.text())
      .then(console.log);
  }


  const [msg, setMsg] = useState(null);
  const msg_feed = []; // Temporary until we establish a real db

  useEffect(() => {
    let isMounted = true;
    const startPolling = () => {
      axios.get("http://localhost:8080")
        .then(response => {
          if (isMounted) {
            setMsg(response.data);
            msg_feed.push(response.data);
            console.log("Message received:", response.data);
            startPolling();
          }
        })
        .catch(error => {
          console.error("Error while polling:", error);
          if (isMounted) {
            setTimeout(startPolling, 8000);
          }
        });
    };

    startPolling();

    return () => { // Cleanup on component unmount
      isMounted = false;
    }
  }, []);

  // TODO: - Parse the message to only keep the relevant part
  // - Display that relevant part
  // - Save that part to a session persistent storage 
  // - Manage the database on the server side IN C!!!!

  return (
    <>
      <div className="div_messages_view">
        <button onClick={receiveMessage}>Get Message</button>
      </div>
      <div>
        <ul>
          <li>Message: {msg}</li>
        </ul>
      </div>
    </>
  )
}

export default MessageDisplay

// useEffect(() => {
//   // Note the "ws://" protocol scheme
//   const socket = new WebSocket("ws://localhost:8080");
//
//   // Connection opened
//   socket.addEventListener("open", (event) => {
//     console.log("WebSocket connected!");
//     socket.send("Hello Server, from React!"); // How you send a message
//   });
//
//   // Listen for messages from the server
//   socket.addEventListener("message", (event) => {
//     console.log("Message from server: ", event.data);
//     // Here you would setMsg(event.data)
//   });
//
//   // Handle closing
//   socket.addEventListener("close", (event) => {
//     console.log("WebSocket disconnected.");
//   });
//
//   // Cleanup on component unmount
//   return () => {
//     socket.close();
//   };
// }, []);
