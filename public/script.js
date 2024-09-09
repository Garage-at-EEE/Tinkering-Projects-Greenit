

// Initialize Firebase
const app = firebase.initializeApp(firebaseConfig);
const database = firebase.database();

// Get a reference to the database node
const dataRef = database.ref('/');

// Function to display data
function displayData() {
    dataRef.on('value', (snapshot) => {
        const data = snapshot.val();
        console.log(data);

        document.getElementById('tdsValue').textContent = data.TDS.float;
        document.getElementById('tempValue').textContent = data.TemperatureC.float;
        document.getElementById('pHValue').textContent = data.pH.float;

        document.getElementById('expectedTdsInput').value = data.ExpectedTDS.float;
        document.getElementById('expectedPHInput').value = data.ExpectedPH.float;
    });
}

// Function to update data
function updateData() {
    const expectedTds = parseFloat(document.getElementById('expectedTdsInput').value);
    const expectedPH = parseFloat(document.getElementById('expectedPHInput').value);

    dataRef.update({
        ExpectedTDS: { float: expectedTds },
        ExpectedPH: { float: expectedPH }
    })
        .then(() => {})
        .catch((error) => {
            console.error('Error updating data:', error);
        });
}

// Load data on page load
window.onload = () => {
    displayData();
};

// Notification when value change
document.getElementById('tdsValue').onchange = (e) => {
    console.log(e) // Arrggg still working on this one
}