import React from "react";
import useDeviceData from "./hooks/useDeviceData";
import LineChart from "./LineChart";

const Graph = ({deviceID}) => {
    const data = useDeviceData(deviceID);

    return (<>
    <h3>Graphs</h3>
      <div className="charts-container">
        <div className="single-chart-container">
          <LineChart
            labels={data?.measure.map(entry => entry.timeStamp) || []}
            data={data?.measure.map(entry => entry.TemperatureC) || []}
            label="Temperature (C)"
            borderColor="rgb(255, 99, 132)"
          />
        </div>
        <div className="single-chart-container">
          <LineChart
            labels={data?.measure.map(entry => entry.timeStamp) || []}
            data={data?.measure.map(entry => entry.TDS) || []}
            label="TDS"
            borderColor="rgb(54, 162, 235)"
          />
        </div>
        <div className="single-chart-container">
          <LineChart
            labels={data?.measure.map(entry => entry.timeStamp) || []}
            data={data?.measure.map(entry => entry.pH) || []}
            label="pH"
            borderColor="rgb(75, 192, 192)"
          />
        </div>
      </div>
    </>);
}

export default Graph;