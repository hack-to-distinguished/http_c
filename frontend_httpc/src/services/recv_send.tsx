// Service to receive and send messages - Will then be used by the
// message box (send messages) and the message feed (recv msg)

export const handleMessage = async ({socket, setMessages, event: MessageEvent}) => {
  if (!socket?.current) return;

  if (socket.current) {
    const receivedMessage = event.data;
    console.log("Message from server:", receivedMessage);
    setMessages((prevMessages) => [...prevMessages, receivedMessage]);

    socket.current.onmessage = handleMessage;
    return () => {
      if (socket.current) {
        socket.current.onmessage = null;
      }
    };
  }
};


export const sendMessage = async ({ socket, currentMessage, setMessages }) => {
  if (currentMessage && socket.current?.readyState === 1) {
    socket.current.send(currentMessage);
    console.log(`Sent message: ${currentMessage}`);
  } else {
    console.log(
      "Cannot send message. WebSocket is not open or message is empty.",
    );
  }
};
