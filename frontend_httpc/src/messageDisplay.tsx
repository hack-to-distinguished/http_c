import { useState, useRef, useEffect } from "react";
import "./components.css";
import axios from "axios";

function MessageDisplay() {

  const serverUrl = "http://127.0.0.1:8080";
  async function receiveMessage() {
    await fetch(serverUrl, {
      method: "GET",
      headers: { "Content-Type": "text/plain" },
    })
      .then(res => res.text())
      .then(console.log);
  }


  const [data, setData] = useState(null);

  useEffect(() => {
    let isMounted = true;
    const startPolling = () => {
      axios.get("http://localhost:8080")
        .then(response => {
          if (isMounted) {
            setData(response.data);
            console.log("Data received:", response.data);
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
    };
  }, []);

  return (
    <>
      <div className="div_messages_view">
        <button onClick={receiveMessage}>Get Message</button>
      </div>
    </>
  )
}

export default MessageDisplay
