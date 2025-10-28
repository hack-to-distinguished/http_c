import { useState, useEffect } from "react";
import "./messageFeed.css";

interface MessageBoxProps {
  socket: React.RefObject<WebSocket | null>;
  connectionStatus: string;
}

const MessageFeed = ({ socket }: MessageBoxProps) => {
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

  // const handleGetMessage = async (): Promise<void> => {
  //   let ws = new WebSocket("ws://127.0.0.1/8080");
  //   console.log("Button Pressed - ws used:", ws);
  //   try {
  //     ws.onopen = () => {
  //       console.log("Connected to ws");
  //       ws.send("connected");
  //     }
  //
  //     // const response = await fetch(ws);
  //     // const newMessages = await response.json();
  //     // setMessages[(prevMessages => [...prevMessages, newMessages])];
  //     // console.log("Results returned", newMessages);
  //
  //   } catch (error) {
  //
  //     console.error("Unable to read data")
  //
  //   }
  // }

  return (
    <div className="messages-display">
      {/* <button */}
      {/*   onPress={handleGetMessage()} */}
      {/* > */}
      {/*   Get messages */}
      {/* </button> */}
      <div className="message-header">HTTP_C Chat</div>
      <ul className="messages-list">
        {messages.length === 0 ? (
          <li className="empty-message">No messages yet.</li>
        ) : (
          messages.map((msg, i) => (
            <li key={i}>
              <span style={{ color: "#003366", fontWeight: "bold" }}>
                {new Date().toLocaleTimeString()} - Message {i + 1}:
              </span>{" "}
              {msg}
            </li>
          ))
        )}
      </ul>
    </div>
  );
};

export default MessageFeed;
