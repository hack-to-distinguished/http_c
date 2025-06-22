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



  // const intervalRecv = (callback, interval, immediate) => {
  //   const ref = useRef();
  //
  //   // keep a reference to the callback without restarting the interval
  //   useEffect(() => {
  //     ref.current = callback;
  //   }, [callback]);
  //
  //   useEffect(() => {
  //     // when this flag is set, closure is stale
  //     let cancelled = false;
  //
  //     // wrap callback to pass isCancelled getter as argument
  //     const fn = () => {
  //       ref.current(() => cancelled);
  //     };
  //
  //     // set interval and run immediately if requested
  //     const id = setInterval(fn, interval);
  //     if (immediate) fn();
  //
  //     // define cleanup logic to run when the component is unmounting
  //     // or when interval or immediate has changed - 
  //     // Executed automatically when the component unmouts or useEffect re-runs
  //     return () => {
  //       cancelled = true;
  //       clearInterval(id);
  //     };
  //   }, [interval, immediate]);
  // };
  //
  // const [message, setMessage] = useState();
  //
  // // This function call has as its first argument an entire function.
  // // The function makes the api call and checks if it has been cancelled.
  // intervalRecv(async (isCancelled) => {
  //   try {
  //     const response = await fetch(serverUrl);
  //     if (isCancelled()) return;
  //
  //     if (response.status != 200) {
  //       throw new Error(response.statusText);
  //     }
  //
  //     console.log("full response:", await response);
  //     const [{ message }] = await response.json();
  //     console.log("is it cancelled:", isCancelled);
  //     if (isCancelled()) return;
  //
  //     console.log("Parsed message:", message);
  //     setMessage(message);
  //   } catch (err) {
  //     console.log("Fetch error:", err);
  //   }
  // }, 15000, true);
  // // }, 1500, true);


  // INFO: Long polling
  const [data, setData] = useState(null);

  useEffect(() => {
    let isMounted = true;
    const startPolling = () => {
      axios.get("http://localhost:8080")
        .then(response => {
          if (isMounted) {
            setData(response.data);
            console.log("Data received:", response.data);
            startPolling();  // Continue polling after receiving data
          }
        })
        .catch(error => {
          console.error("Error while polling:", error);
          if (isMounted) {
            setTimeout(startPolling, 8000);  // Retry after a delay on error
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
