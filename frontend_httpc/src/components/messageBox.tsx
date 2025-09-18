import { useState, useEffect } from "react";
import { sendMessage, recvMessage } from "../services/recv_send.tsx";
import "./messageBox.css";

interface MessageBoxProps {
  socket: WebSocket;
  connectionStatus: string;
}

const MessageBox = ({ socket, connectionStatus }: MessageBoxProps) => {
  const [messages, setMessages] = useState<string[]>([]);
  const [currentMessage, setCurrentMessage] = useState<string>("");

  // useEffect(() => {
  //   const handleRecvMessage = async () => {
  //     // e.preventDefault();
  //     await recvMessage({ socket, setMessages });
  //   };
  //   handleRecvMessage();
  //
  //   const interval = setInterval(handleRecvMessage, 1000);
  //
  //   return () => {
  //     clearInterval(interval);
  //   };
  // }, [socket]);

  // useEffect(() => {
  //   recvMessage({ socket, setMessages });
  //
  //   return () => {
  //     if (socket?.current) {
  //       socket.current.onmessage = null;
  //     }
  //   };
  // }, [socket]);


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

  const handleSendMessage = async (e) => {
    try {
      e.preventDefault();
      await sendMessage({ socket, currentMessage });
      setCurrentMessage("");
    } catch (e) {
      console.log(`Error in sendMessage function: ${e}`);
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
          className="msg-input-box" type="text"
          value={currentMessage} placeholder="Type a message..."
          onChange={(e) => setCurrentMessage(e.target.value)}
        />
        <button
          type="submit"
          disabled={connectionStatus !== "Connected"}
          className="send-msg-button"> Send Message
        </button>
      </form>
    </div>
  );
}

export default MessageBox;
