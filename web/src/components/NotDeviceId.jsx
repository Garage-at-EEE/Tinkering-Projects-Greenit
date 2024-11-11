import { useState } from "react";
import "../style/credentials.css";
import { database, ref } from "./firebaseConfig";
import { get, child } from "firebase/database";
import SavedDevices from "./SavedDevices";

const NotDeviceId = ({ deviceIdState }) => {
    window.scrollTo({ top: 0, behavior: 'smooth' });

    const [deviceID, setDeviceID] = deviceIdState;
    const [inputID, setInputID] = useState("");
    const [inputPassword, setInputPassword] = useState("");
    const [isStay, setIsStay] = useState(false);
    const [isSaveDevice, setIsSaveDevice] = useState(false);
    const [deviceAlias, setDeviceAlias] = useState("");

    const handleIfStayLogIn = () => {
        setIsStay(!isStay);
    };

    const handleIfSaveDevice = () => {
        setIsSaveDevice(!isSaveDevice);
    }

    // Function to check if device ID exists
    async function checkDeviceValid(inputID, inputPassword) {
        console.log(inputID);

        const dbRef = ref(database, '/devices/');

        try {
            const snapshot = await get(child(dbRef, inputID));

            if (snapshot.exists()) {
                console.log("Key exists:");

                const device = snapshot.val();
                if (inputPassword != device.password) {
                    alert("Incorrect password");
                    return false;
                }

                alert("Validate successfully!");
                if (isStay) localStorage.setItem("deviceID", inputID);

                if (isSaveDevice){
                    var savedDevicesString = localStorage.getItem("saved-devices");
                    var savedDevicesJSON = [];
                    if (savedDevicesString) savedDevicesJSON = JSON.parse(savedDevicesString);
                    else savedDevicesString = "";
                    savedDevicesJSON.push({
                        deviceID: inputID,
                        deviceAlias: deviceAlias
                    });
                    console.log(savedDevicesJSON);
                    savedDevicesString = JSON.stringify(savedDevicesJSON);
                    localStorage.setItem("saved-devices", savedDevicesString);
                }
                console.log(localStorage.getItem("saved-devices"));

                setDeviceID(inputID);
                return true;
            } else {
                alert("Invalid device ID");
                return false;
            }
        } catch (error) {
            alert("Error checking device ID:", error);
            return false;
        }
    }

    return (<>
        <h3>Device credentials:</h3>

        <table>
            <tbody>
                <tr className="item-form">
                    <td className="value_label">Device ID:</td>
                    <td><input className="num-input credential" type="text" onChange={(e) => setInputID(e.target.value)} /></td>
                </tr>
                <tr className="item-form">
                    <td className="value_label">Password:</td>
                    <td><input className="num-input credential" type="text" onChange={(e) => setInputPassword(e.target.value)} /></td>
                </tr>
            </tbody>
        </table>

        <div className="checkbox-wrapper-28">
            <input
                id="tmp-28"
                type="checkbox"
                className="promoted-input-checkbox"
                checked={isStay}
                onChange={handleIfStayLogIn}
            />
            <svg><use xlinkHref="#checkmark-28" /></svg>
            <label htmlFor="tmp-28">
                Stay logged in this device
            </label>
            <svg xmlns="http://www.w3.org/2000/svg" style={{ display: "none" }}>
                <symbol id="checkmark-28" viewBox="0 0 24 24">
                    <path
                        strokeLinecap="round"
                        strokeMiterlimit="10"
                        fill="none"
                        d="M22.9 3.7l-15.2 16.6-6.6-7.1"
                    />
                </symbol>
            </svg>
        </div>

        <div className="checkbox-wrapper-28">
            <input
                id="tmp-29"
                type="checkbox"
                className="promoted-input-checkbox"
                checked={isSaveDevice}
                onChange={handleIfSaveDevice}
            />
            <svg><use xlinkHref="#checkmark-28" /></svg>
            <label htmlFor="tmp-29">
                Save this device
            </label>
            <svg xmlns="http://www.w3.org/2000/svg" style={{ display: "none" }}>
                <symbol id="checkmark-29" viewBox="0 0 24 24">
                    <path
                        strokeLinecap="round"
                        strokeMiterlimit="10"
                        fill="none"
                        d="M22.9 3.7l-15.2 16.6-6.6-7.1"
                    />
                </symbol>
            </svg>
        </div>

        {(isSaveDevice) ? (<div className="hidden-field">
            <td className="value_label">Device alias:</td>
            <td><input className="num-input credential alias" type="text" onChange={(e) => setDeviceAlias(e.target.value)} /></td>
        </div>) : (<></>)}

        <button className="action" onClick={(e) => checkDeviceValid(inputID, inputPassword)}>Validate</button>

        <p>
            Device ID is a 6-digit number. Password is an 8-alpha-numerical-character string.
            Both can be found at the bottom of your device.
        </p>

        <h3>Saved devices</h3>
        <SavedDevices setDeviceID={setDeviceID}/>
    </>);
}

export default NotDeviceId;