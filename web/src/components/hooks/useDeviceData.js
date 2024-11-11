import { useEffect, useState } from "react";
import { database, ref, onValue } from "../firebaseConfig";

const useDeviceData = (deviceID) => {
  const [data, setData] = useState(null);

  useEffect(() => {
    const dataRef = ref(database, `/devices/${deviceID}/`);
    const unsubscribe = onValue(dataRef, (snapshot) => {
      setData(snapshot.val());
    });

    return () => unsubscribe();
  }, [deviceID]);

  return data;
};

export default useDeviceData;
