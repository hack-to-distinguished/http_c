import { useState, useRef, useEffect } from "react"
import "./components.css"

function MessageDisplay() {
  // This component will be used to display the messages.
  // The MessageBox page will call the C API and here we handle the displaying of each message.
  // They should be displayed from bottom to top as if coming from below the screen.
  // They slowly float to the top, they aren't forever, only available for a short period of time.
  // If no messages are sent for a while, the most recent messages (maybe 10 max) float back down and wait.
  // When a messages is sent, they're pushed back up.
  // The web view of HTTPC is more for fun (this one at least). The actual important messaging section with 
  // extra security and stuff will be through a different medium. I'm thinking something SSH related. Accessible via
  // the command line or something else that would be easily portable to a mobile device. All security will be handled
  // on the sever side (C) so doing the web client this way doesn't weaken the important part

  const serverUrl = "http://127.0.0.0:8080";
  async function receiveMessage(event) {
    await fetch("http://127.0.0.0:8080", {
      method: "GET",
      headers: {"Content-Type": "text/plain" },
    })
    .then(res => res.text())
    .then(console.log);
  }

  const intervalRecv = (callback, interval, immediate) => {
    const ref = useRef();

    // keep a reference to the callback without restarting the interval
    useEffect(() => {
      ref.current = callback;
    }, [callback]);

    useEffect(() => {
      // when this flag is set, closure is stale
      let cancelled = false;

      // wrap callback to pass isCancelled getter as argument
      const fn = () => {
        ref.current(() => cancelled);
      };

      // set interval and run immediately if requested
      const id = setInterval(fn, interval);
      if (immediate) fn();

      // define cleanup logic to run when the component is unmounting
      // or when interval or immediate has changed
      return () => {
        cancelled = true;
        clearInterval(id);
      };
    }, [interval, immediate]);
  };

  const [message, setMessage] = useState();

  intervalRecv(async (isCancelled) => {
    try {
      const response = await fetch(serverUrl);
      if (isCancelled()) return;

      if (response.status != 200) {
        throw new Error(response.statusText);
      }

      console.log("full response:", await response);
      const [{message}] = await response.json();
      if (isCancelled) return;

      console.log(message);
      setMessage(message);
    } catch (err) {
      console.log("Fetch error:", err);
    }
  }, 15000, true);
  // }, 1500, true);

  return (
    <>
      <div className="div_messages_view">
        <button onClick={receiveMessage}>Get Message</button>
      </div>
    </>
  )
}

export default MessageDisplay
