import { useState } from 'react'
import './components.css'

function MessageBox() {

  return (
    <>
      <div class="div_input_message">
        <label class="label_input_message">What are you thinking?</label>
        <input type="text" id="message" size="40" class="input_message_box"/>
      </div>
    </>
  )
}

export default MessageBox
