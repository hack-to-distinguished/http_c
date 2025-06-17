import React, { useState } from 'react';
import './components.css'

function MessageBox() {
  // Somehow call the C function, API or whatever from here
  const [msg, setMsg] = useState(""); 

  function handleOnChange(event) {
    setMsg(event.target.value);
  }

  function handleFormSubmit(event) {
    event.preventDefault();
    console.log("Message sent:", event.currentTarget.elements.messageInput.value);

    // fetch('http://127.0.0.0:8080', {
    //   method: 'POST',
    //   headers: { 'Content-Type': 'text/plain' },
    //   body: 'Hello from browser'
    // })
    //   .then(res => res.text())
    //   .then(console.log);
    setMsg("");
  }

  return (
    <>
      <form onSubmit={handleFormSubmit}>
        <div className="div_input_message">
          <label className="label_input_message">What are you thinking?</label>
          <input type="text" id="messageInput" value={msg} onChange={handleOnChange} className="input_message_box"/>
        </div>
      </form>
    </>
  )
}

export default MessageBox
