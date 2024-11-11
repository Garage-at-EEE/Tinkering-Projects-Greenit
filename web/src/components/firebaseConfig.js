import firebaseConfig from "./secret";
import { initializeApp } from "firebase/app";
import { getDatabase, ref, onValue, update } from "firebase/database";

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

export { database, ref, onValue, update };