import React, { useState, useEffect } from "react";
import { database, ref, update } from "./firebaseConfig";
import useDeviceData from "./hooks/useDeviceData";

const UpdateValue = ({ deviceID }) => {
    const data = useDeviceData(deviceID);

    const [expectedTDS, setExpectedTDS] = useState("0");
    const [expectedPH, setExpectedPH] = useState("0");

    useEffect(() => {
        if (data) {
            setExpectedTDS(data.expected?.ExpectedTDS || "0");
            setExpectedPH(data.expected?.ExpectedPH || "0");
        }
    }, [data]);

    const handleUpdate = () => {
        const dataRef = ref(database, `/devices/${deviceID}/`);
        update(dataRef, {
            expected: {
                ExpectedTDS: parseFloat(expectedTDS),
                ExpectedPH: parseFloat(expectedPH),
            },
        })
            .then(() => alert("Updated successfully!"))
            .catch((error) => console.error("Error updating data:", error));
    };

    return (
        <>
            <h3>Update Expected Values</h3>
            <table>
                <tbody>
                    <tr className="item-form">
                        <td className="value_label">Expected TDS:</td>
                        <td><input className="num-input" type="number" value={expectedTDS} onChange={(e) => setExpectedTDS(e.target.value)} /></td>
                    </tr>
                    <tr className="item-form">
                        <td className="value_label">Expected pH:</td>
                        <td><input className="num-input" type="number" value={expectedPH} onChange={(e) => setExpectedPH(e.target.value)} /></td>
                    </tr>
                </tbody>
            </table>
            <button className="action" onClick={handleUpdate}>Update</button>
        </>
    );
}

export default UpdateValue;