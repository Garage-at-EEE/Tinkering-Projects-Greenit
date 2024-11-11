import React, { useState, useEffect } from 'react';
import useDeviceData from "./hooks/useDeviceData";

function DataDisplay({ deviceID }) {
  const data = useDeviceData(deviceID);

  const [previousValues, setPreviousValues] = useState({ TDS: null, TemperatureC: null, pH: null });
  const [animationClass, setAnimationClass] = useState({ TDS: '', TemperatureC: '', pH: '' });

  useEffect(() => {
    // Check for changes in TDS
    if (data?.measure[9]?.TDS !== previousValues.TDS) {
      setAnimationClass((prev) => ({ ...prev, TDS: 'highlight' }));
      setTimeout(() => setAnimationClass((prev) => ({ ...prev, TDS: '' })), 1000);
    }

    // Check for changes in TemperatureC
    if (data?.measure[9]?.TemperatureC !== previousValues.TemperatureC) {
      setAnimationClass((prev) => ({ ...prev, TemperatureC: 'highlight' }));
      setTimeout(() => setAnimationClass((prev) => ({ ...prev, TemperatureC: '' })), 1000);
    }

    // Check for changes in pH
    if (data?.measure[9]?.pH !== previousValues.pH) {
      setAnimationClass((prev) => ({ ...prev, pH: 'highlight' }));
      setTimeout(() => setAnimationClass((prev) => ({ ...prev, pH: '' })), 1000);
    }

    // Update previous values
    setPreviousValues({
      TDS: data?.measure[9]?.TDS,
      TemperatureC: data?.measure[9]?.TemperatureC,
      pH: data?.measure[9]?.pH,
    });
  }, [data]);

  return (<>
    <h3>Real-time database</h3>
    <table>
      <tbody>
        <tr className="item">
          <td className="value_label">TDS:</td>
          <td><input className={`data-field ${animationClass.TDS}`} type="text" value={data?.measure[9]?.TDS || "0"} readOnly /></td>
        </tr>
        <tr className="item">
          <td className="value_label">Temperature (C):</td>
          <td><input className={`data-field ${animationClass.TemperatureC}`} type="text" value={data?.measure[9]?.TemperatureC || "0"} readOnly /></td>
        </tr>
        <tr className="item">
          <td className="value_label">pH:</td>
          <td><input className={`data-field ${animationClass.pH}`} type="text" value={data?.measure[9]?.pH || "0"} readOnly /></td>
        </tr>
      </tbody>
    </table>
  </>
  );
}

export default DataDisplay;
