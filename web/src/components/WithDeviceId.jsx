import DataDisplay from "./DataDisplay";
import UpdateValue from "./UpdateValue";
import Graph from "./Graph";

const WithDeviceID = ({ deviceID, setDeviceID }) => {
    window.scrollTo({ top: 0, behavior: 'smooth' });
    
    function logout(){
        // eslint-disable-next-line no-restricted-globals
        if (confirm("Are you sure to log out?")){
            localStorage.setItem("deviceID", "");
            setDeviceID("");
        }
    }

    return (<>
        {/* Display Values */}
        <DataDisplay deviceID={deviceID} />

        {/* Update Expected Values */}
        <UpdateValue deviceID={deviceID} />

        {/* Graphs for Temperature, TDS, and pH */}
        <Graph deviceID={deviceID} />

        {/* Log out from device */}
        <h3>Log out from device</h3>
        <button className="action danger" onClick={logout}>Log out</button>
    </>);
}

export default WithDeviceID;