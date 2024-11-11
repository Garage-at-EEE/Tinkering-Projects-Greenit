import React, { useState, useEffect } from "react";

import "../style/style.css";
import WithDeviceID from "./WithDeviceId";
import NotDeviceId from "./NotDeviceId";

const App = () => {
  const [deviceID, setDeviceID] = useState("");

  // Retrieve the value from localStorage and set it as initial state
  useEffect(() => {
    const savedDeviceID = localStorage.getItem("deviceID");
    if (savedDeviceID) {
      setDeviceID(savedDeviceID);
    }
  }, []); // Empty dependency array to run only once on mount

  return (
    <div className="container">
      <h1 className="header">Green It! Control Panel</h1>

      {deviceID ? (<WithDeviceID deviceID={deviceID} setDeviceID={setDeviceID} />) : (<NotDeviceId deviceIdState={[deviceID, setDeviceID]} />)}
    </div>
  );
};

export default App;
