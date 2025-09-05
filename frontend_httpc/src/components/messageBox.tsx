import { useState } from "react";
import { sendMessage, recvMessage } from "../services/recv_send.tsx";
import "./messageBox.css";

function MessageBox({ socket, connectionStatus }) {
  const [messages, setMessages] = useState([]);
  const [currentMessage, setCurrentMessage] = useState("");

  useEffect(() => {
    const handleRecvMessage = async () => {
      e.preventDefault();
      await recvMessage({ socket });
    }
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
