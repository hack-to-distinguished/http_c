import "./App.css";
import MessageDisplay from "./pages/messageDisplay.tsx";

function App() {
  return (
    <>
      <h1 className="head_title">HTTP_C CHAT</h1>
      {/* The div below should contain the chat feed of floating messages */}
      {/*Should also contain the enter message chat box */}
      <div className="main-app-container">
        <MessageDisplay />
      </div>
    </>
  );
}

export default App;
