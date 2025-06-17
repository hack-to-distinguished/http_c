import { useState } from 'react'
import './components.css'

function MessageDisplay() {
  // This component will be used to display the messages.
  // The MessageBox page will call the C API and here we handle the displaying of each message.
  // They should be displayed from bottom to top as if coming from below the screen.
  // They slowly float to the top, they aren't forever, only available for a short period of time.
  // If no messages are sent for a while, the most recent messages (maybe 10 max) float back down and wait.
  // When a messages is sent, they're pushed back up.
  // The web view of HTTPC is more for fun (this one at least). The actual important messaging section with 
  // extra security and stuff will be through a different medium. I'm thinking something SSH related. Accessible via
  // the command line or something else that would be easily portable to a mobile device. All security will be handled
  // on the sever side (C) so doing the web client this way doesn't weaken the important part

  return (
    <>
      <div className="div_messages_view">
      </div>
    </>
  )
}

export default MessageDisplay
