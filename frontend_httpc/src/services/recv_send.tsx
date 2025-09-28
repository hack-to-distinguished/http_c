// Service to receive and send messages - Will then be used by the
// message box (send messages) and the message feed (recv msg)

// export const recvMessage = async ({ socket, setMessages }) => {
//   // TODO: Run on a setTimeout interval
// 	socket.current.onmessage = (event) => {
// 		const receivedMessage = event.data;
// 		console.log("Message from server: ", receivedMessage);
//
// 		// Add the new message to our list of messages
// 		// Use a functional update to get the previous state correctly
// 		setMessages(prevMessages => [...prevMessages, receivedMessage]);
//
// 	};
//
// };

export const recvMessage = async ({ socket, setMessages }) => {
  if (!socket?.current) return;

  if (!socket.current.onmessage) {
    socket.current.onmessage = (event) => {
      const receivedMessage = event.data;
      console.log("Message from server:", receivedMessage);
      setMessages(prevMessages => [...prevMessages, receivedMessage]);
    };
  }
};

export const sendMessage = async ({ socket, currentMessage }) => {
  if (currentMessage && socket.current?.readyState === 1) {
	socket.current.send(currentMessage);
    console.log(`Sent message: ${currentMessage}`);
  } else {
    console.log(
      "Cannot send message. WebSocket is not open or message is empty.",
    );
  }
};
