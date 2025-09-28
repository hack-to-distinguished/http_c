import { useState } from "react";
import { sendMessage } from "../services/recv_send.tsx";
import "./messageBox.css";

interface MessageBoxProps {
  socket: WebSocket;
  connectionStatus: string;
}

const MessageBox = ({ socket, connectionStatus }: MessageBoxProps) => {
  const [currentMessage, setCurrentMessage] = useState<string>("");

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
    <div className="send-msg-container">
      <form onSubmit={handleSendMessage} className="message-form">
        <div className="form-heading">Message Input</div>
        <input
          className="msg-input-box"
          type="text"
          value={currentMessage}
          placeholder="Type a message..."
          onChange={(e) => setCurrentMessage(e.target.value)}
        />
        <button
          type="submit"
          disabled={connectionStatus !== "Connected"}
          className="send-msg-button"
        >
          Send Message
        </button>
        <div
          style={{
            textAlign: "left",
            marginTop: "10px",
            fontSize: "10px",
            color: "#666",
          }}
        ></div>
      </form>
    </div>
  );
};

export default MessageBox;
