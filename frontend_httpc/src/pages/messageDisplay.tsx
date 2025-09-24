import { useWebSocket } from "../services/network.tsx";
import NetworkStatus from "../components/networkStatus.tsx";
import MessageBox from "../components/messageBox.tsx";
import MessageFeed from "../components/messageFeed.tsx";
import "./messageDisplay.css";

function MessageDisplay() {
  const serverUrl = "ws://127.0.0.1:8080";
  const { socket, connectionStatus } = useWebSocket(serverUrl);

  return (
    <>
      <NetworkStatus connectionStatus={connectionStatus} />
      <MessageFeed socket={socket} connectionStatus={connectionStatus} />
      <MessageBox socket={socket} connectionStatus={connectionStatus} />
    </>
  );
}

export default MessageDisplay;
