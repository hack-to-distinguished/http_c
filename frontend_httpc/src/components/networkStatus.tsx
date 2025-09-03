function NetworkStatus({ socket, connectionStatus }) {
  return (
    <div className="status">
      <strong>Connection Status:</strong> {connectionStatus}
    </div>
  );
}

export default NetworkStatus;
