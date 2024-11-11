const SavedDeviceCard = ({ deviceAlias, deviceID, setDeviceID, removeDevice }) => {
    const login = () => {
        // Set device ID in localStorage and trigger login
        localStorage.setItem("deviceID", deviceID);
        setDeviceID(deviceID); // Update the deviceID in the parent component state
    };

    return (
        <div className="saved-device-card">
            <div className="saved-device-alias">{deviceAlias}</div>
            <div className="saved-device-id">{deviceID}</div>
            <button className="action btn-in-card" onClick={login}>Log in</button>
            <button className="action danger" onClick={removeDevice}>Remove</button>
        </div>
    );
};

export default SavedDeviceCard;
