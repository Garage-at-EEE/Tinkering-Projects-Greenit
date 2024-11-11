import React, { useState, useEffect } from "react";
import { database, ref, update } from "./firebaseConfig";
import useDeviceData from "./hooks/useDeviceData";

const UpdateValue = ({ deviceID }) => {
    const data = useDeviceData(deviceID);

    const [expectedTDS, setExpectedTDS] = useState("0");
    const [expectedPH, setExpectedPH] = useState("0");

    // Preset options for pH and TDS values
    const presets = [
        { label: "Option 1 - pH 6.0, TDS 500", ph: 6.0, tds: 500 },
        { label: "Option 2 - pH 5.5, TDS 600", ph: 5.5, tds: 600 },
        { label: "Option 3 - pH 6.5, TDS 700", ph: 6.5, tds: 700 },
    ];

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

    const handlePresetChange = (event) => {
        const selectedPreset = presets[event.target.value];
        setExpectedPH(selectedPreset.ph);
        setExpectedTDS(selectedPreset.tds);
        // Update database immediately after selecting a preset
        const dataRef = ref(database, `/devices/${deviceID}/`);
        update(dataRef, {
            expected: {
                ExpectedTDS: selectedPreset.tds,
                ExpectedPH: selectedPreset.ph,
            },
        })
            .then(() => alert("Preset updated successfully!"))
            .catch((error) => console.error("Error updating data:", error));
    };

    return (
        <>
            <h3>Update Expected Values</h3>
            <table>
                <tbody>
                    <tr className="item-form">
                        <td className="value_label">Expected TDS:</td>
                        <td>
                            <input
                                className="num-input"
                                type="number"
                                value={expectedTDS}
                                onChange={(e) => setExpectedTDS(e.target.value)}
                            />
                        </td>
                    </tr>
                    <tr className="item-form">
                        <td className="value_label">Expected pH:</td>
                        <td>
                            <input
                                className="num-input"
                                type="number"
                                value={expectedPH}
                                onChange={(e) => setExpectedPH(e.target.value)}
                            />
                        </td>
                    </tr>
                </tbody>
            </table>
            <button className="action" onClick={handleUpdate}>Update</button>

            <h4>Preset Options</h4>
            <select onChange={handlePresetChange} defaultValue="" className="dropdown">
                <option value="" disabled>Select a preset</option>
                {presets.map((preset, index) => (
                    <option key={index} value={index}>
                        {preset.label}
                    </option>
                ))}
            </select>
        </>
    );
};

export default UpdateValue;
