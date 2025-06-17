import { useState } from 'react'
import './App.css'
import MessageBox from './messageBox.tsx'

function App() {

  return (
    <>
      <h1 className="head_title">Web Chat View</h1>
      {/* The div below should contain the chat feed of floating messages */}
      {/*Should also contain the enter message chat box */}
      <div>
        <MessageBox/>
      </div>
    </>
  )
}

export default App
