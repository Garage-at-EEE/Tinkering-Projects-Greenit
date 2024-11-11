import { useEffect, useRef } from "react";
import Chart from "chart.js/auto";

const LineChart = ({ labels, data, label, borderColor }) => {
  const chartRef = useRef(null);

  useEffect(() => {
    const ctx = chartRef.current.getContext("2d");
    const chartInstance = new Chart(ctx, {
      type: "line",
      data: {
        labels,
        datasets: [{ label, data, borderColor, fill: false }],
      },
    });

    return () => chartInstance.destroy();
  }, [labels, data, label, borderColor]);

  return <canvas ref={chartRef} />;
};

export default LineChart;
