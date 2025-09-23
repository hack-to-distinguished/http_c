import "./networkStatus.css"
function NetworkStatus({ connectionStatus }) {
  return (
    <div className="network-status-message">
      <strong>Connection Status:</strong> {connectionStatus}
    </div>
  );
}

export default NetworkStatus;
