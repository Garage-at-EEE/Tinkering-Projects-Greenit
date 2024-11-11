import { useState, useEffect } from "react";
import SavedDeviceCard from "./SavedDeviceCard";

const SavedDevices = ({ setDeviceID }) => {
    const [savedDevices, setSavedDevices] = useState([]);

    // Load saved devices from localStorage when component mounts
    useEffect(() => {
        const savedDevicesString = localStorage.getItem("saved-devices");
        if (savedDevicesString) {
            const savedDevicesJSON = JSON.parse(savedDevicesString);
            setSavedDevices(savedDevicesJSON);
        }
    }, []);

    // Remove device from saved devices list
    function removeDevice(index) {
        if (window.confirm("Are you sure to remove this device?")) {
            const newSavedDevicesJSON = savedDevices.filter((_, i) => i !== index);
            localStorage.setItem("saved-devices", JSON.stringify(newSavedDevicesJSON));
            setSavedDevices(newSavedDevicesJSON);  // Update state to trigger re-render
        }
    }

    return savedDevices.length ? (
        <>
            <p>Saved devices:</p>
            <div className="saved-devices-container">
                {savedDevices.slice().reverse().map((device, index) => (
                    <SavedDeviceCard
                        key={index}
                        index={index}
                        deviceAlias={device.deviceAlias}
                        deviceID={device.deviceID}
                        setDeviceID={setDeviceID}
                        removeDevice={() => removeDevice(savedDevices.length - index - 1)}  // Correct index already
                    />
                ))}
            </div>
        </>
    ) : (
        <p>No saved devices yet.</p>
    );
};

export default SavedDevices;
