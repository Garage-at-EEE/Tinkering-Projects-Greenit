//document.oncontextmenu = (e) => { e.preventDefault() };

// Initialize Firebase
const app = firebase.initializeApp(firebaseConfig);
const database = firebase.database();
const dataRef = database.ref('/');

// Chart.js data arrays and labels
let timeLabels = [];
let tempData = [];
let tdsData = [];
let pHData = [];

// Chart variables
let tempChart, tdsChart, phChart;

// Check for first load
var isFirstLoad = true;

// Function to round number to 2 decimal places
function roundToTwo(num) {
    return Math.round(num * 100) / 100;
}


// Function to initialize the charts after DOM is fully loaded
function initializeCharts() {
    const tempCtx = document.getElementById('tempChart').getContext('2d');
    const tdsCtx = document.getElementById('tdsChart').getContext('2d');
    const phCtx = document.getElementById('phChart').getContext('2d');

    tempChart = new Chart(tempCtx, {
        type: 'line',
        data: {
            labels: timeLabels,
            datasets: [{
                label: 'Temperature (C)',
                data: tempData,
                borderColor: 'rgb(255, 99, 132)',
                fill: false
            }]
        }
    });

    tdsChart = new Chart(tdsCtx, {
        type: 'line',
        data: {
            labels: timeLabels,
            datasets: [{
                label: 'TDS',
                data: tdsData,
                borderColor: 'rgb(54, 162, 235)',
                fill: false
            }]
        }
    });

    phChart = new Chart(phCtx, {
        type: 'line',
        data: {
            labels: timeLabels,
            datasets: [{
                label: 'pH',
                data: pHData,
                borderColor: 'rgb(75, 192, 192)',
                fill: false
            }]
        }
    });
}

// Function to create highlight when a value is changed
function notification(object) {
    object.style.animation = "highlight 1s";
    setTimeout(() => {
        object.style.animation = "none";
    }, 1000);
}

// Function to fetch and display data
function fetchAndDisplayData() {
    dataRef.once('value').then((snapshot) => {
        const data = snapshot.val();
        updateDisplay(data);
    }).catch((error) => {
        console.error('Error fetching data:', error);
    });
}

// Function to update the display and charts with new data
function updateDisplay(data) {
    // Get current time label
    let currentTime = new Date().toLocaleTimeString();

    // Append new data
    timeLabels.push(currentTime);
    tempData.push(data.TemperatureC.float);
    tdsData.push(data.TDS.float);
    pHData.push(data.pH.float);

    const tdsValue = document.getElementById('tdsValue');
    const tempValue = document.getElementById('tempValue');
    const pHValue = document.getElementById('pHValue');

    // Update values on the page and highlight if changed
    if (tdsValue.value != data.TDS.float && !isFirstLoad) notification(tdsValue);
    tdsValue.value = data.TDS.float;

    if (tempValue.value != data.TemperatureC.float && !isFirstLoad) notification(tempValue);
    tempValue.value = data.TemperatureC.float;

    if (pHValue.value != data.pH.float && !isFirstLoad) notification(pHValue);
    pHValue.value = data.pH.float;

    if (isFirstLoad) {
        // Update expected values input fields
        document.getElementById('expectedTdsInput').value = data.ExpectedTDS.float;
        document.getElementById('expectedPHInput').value = data.ExpectedPH.float;
    }

    // Limit to the 10 most recent data points by slicing the arrays
    if (timeLabels.length > 10) {
        timeLabels = timeLabels.slice(-10);
        tempData = tempData.slice(-10);
        tdsData = tdsData.slice(-10);
        pHData = pHData.slice(-10);
    }

    // Reassign chart data with the latest 10 entries and update
    tempChart.data.labels = timeLabels;
    tempChart.data.datasets[0].data = tempData;
    tempChart.update();

    tdsChart.data.labels = timeLabels;
    tdsChart.data.datasets[0].data = tdsData;
    tdsChart.update();

    phChart.data.labels = timeLabels;
    phChart.data.datasets[0].data = pHData;
    phChart.update();

    isFirstLoad = false;
}


// Function to update data
function updateData() {
    const expectedTds = parseFloat(document.getElementById('expectedTdsInput').value);
    const expectedPH = parseFloat(document.getElementById('expectedPHInput').value);

    dataRef.update({
        ExpectedTDS: { float: expectedTds },
        ExpectedPH: { float: expectedPH }
    })
        .then(() => {
            alert("Updated successfully!");
        })
        .catch((error) => {
            console.error('Error updating data:', error);
        });
}

// Initialize charts and start fetching data
window.onload = () => {
    initializeCharts();

    // Poll data every second
    setInterval(fetchAndDisplayData, 1000);

    // Listen for real-time updates from Firebase
    dataRef.on('value', (snapshot) => {
        const data = snapshot.val();
        updateDisplay(data);
        // Update expected values input fields
        document.getElementById('expectedTdsInput').value = data.ExpectedTDS.float;
        document.getElementById('expectedPHInput').value = data.ExpectedPH.float;
    });
};
