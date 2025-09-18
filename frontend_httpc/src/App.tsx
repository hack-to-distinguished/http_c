import "./App.css";
import MessageDisplay from "./pages/messageDisplay.tsx";

function App() {

  return (
    <>
      {/* The div below should contain the chat feed of floating messages */}
      {/*Should also contain the enter message chat box */}
      <div>
        <MessageDisplay />
      </div>
    </>
  )
}

export default App
