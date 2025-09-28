import "./networkStatus.css";
function NetworkStatus({ connectionStatus }) {
  const getStatusClass = () => {
    switch (connectionStatus) {
      case "Connected":
        return "status-connected";
      case "Disconnected":
        return "status-disconnected";
      case "Connecting...":
        return "status-connecting";
      case "Error":
        return "status-error";
      default:
        return "";
    }
  };

  return (
    <div className="network-status-message">
      <strong>Connection Status:</strong>
      <span className={getStatusClass()}>{connectionStatus}</span>
      <div className="connection-date">
        {new Date().toLocaleDateString("en-US", {
          month: "short",
          day: "numeric",
          year: "numeric",
        })}
      </div>
    </div>
  );
}

export default NetworkStatus;
