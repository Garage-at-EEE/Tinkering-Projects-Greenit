document.oncontextmenu = (e) => {e.preventDefault()}

// Initialize Firebase
const app = firebase.initializeApp(firebaseConfig);
const database = firebase.database();

// Get a reference to the database node
const dataRef = database.ref('/');

// Function to create highlight when a value is changed
function notification(object) {
    object.style.animation = "highlight 1s";
    setTimeout(()=>{
        object.style.animation = "none";
    }, 1000);
    
}

// Function to display data
var isFirstLoad = true;
function displayData() {
    dataRef.on('value', (snapshot) => {
        const data = snapshot.val();
        console.log(data);

        const tdsValue = document.getElementById('tdsValue');
        const tempValue = document.getElementById('tempValue');
        const pHValue = document.getElementById('pHValue');

        var tempTdsValue = tdsValue.value;
        if (tempTdsValue != data.TDS.float && !isFirstLoad) notification(tdsValue);
        tdsValue.value = data.TDS.float;

        var tempTempValue = tempValue.value;
        if (tempTempValue != data.TemperatureC.float && !isFirstLoad) notification(tempValue);
        tempValue.value = data.TemperatureC.float;

        var tempPHValue = pHValue.value;
        if (tempPHValue != data.pH.float && !isFirstLoad) notification(pHValue);
        pHValue.value = data.pH.float;

        document.getElementById('expectedTdsInput').value = data.ExpectedTDS.float;
        document.getElementById('expectedPHInput').value = data.ExpectedPH.float;

        isFirstLoad = false;
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
        .then(() => { })
        .catch((error) => {
            console.error('Error updating data:', error);
        });
}

// Load data on page load
window.onload = () => {
    displayData();
};

