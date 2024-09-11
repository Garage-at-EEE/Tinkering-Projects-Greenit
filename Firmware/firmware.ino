#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "garage@eee"
#define WIFI_PASSWORD "garage@eee"

// Insert Firebase project API Key
#define API_KEY "AIzaSyADb27BQUq3PxLYmBmxkkGqlbY0y2HK-Z0"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://tinkering-projects-greenit-default-rtdb.asia-southeast1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;


// "pins"
// GPIO where the DS18B20 is connected to
  
#define TdsSensorPin 36       // Define the analog pin where the pH sensor is connected
#define PH_DOWN_PIN 25
#define PH_UP_PIN 22
#define NUTRIENT_PIN 32
#define WATER_PIN 39
#define flush_PIN 23
#define pH_PIN 34 // Use any analog pin
#define oneWireBus 33 
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
#define SCOUNT  30 
// "arrays"
int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

// "parameters"
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
           // sum of sample point

float averageVoltage = 0;
float tdsValue = 0;
float pHValue = 0.00;

// Thresholds and desired values that will be inputed by Users
float pHf = 6.0; // pH lower bound, f  for floor
float pHc = 7.0; // pH upper bound, c  for ceiling
float ECf = 1.5; // EC lower bound
float ECc = 2.0; // EC upper bound
float water_level_threshold = 2024; // eg threshold for water level

// Time constraints
unsigned long startTime;
unsigned long runDuration = 1728000000; // 20 days in milliseconds
 
 
 // median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

float detect_TDS(float temperature){
    
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U) //every 40 milliseconds,read the analog value from the ADC
  {     
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    // Serial.println("TDS Raw_correct: ");
    // Serial.print(analogBuffer[analogBufferIndex]);
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT)
    { 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 80U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4096.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //temperature compensation
      float compensationVoltage=averageVoltage/compensationCoefficient;
      // Serial.print(compensationVoltage);
      // Serial.print(averageVoltage);
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
      
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      
    }
    Serial.print("TDS Value:");
    Serial.print(tdsValue,1);
    Serial.println("ppm");
  }
  return tdsValue;
}

float detect_pH(){
int sensorValue = analogRead(pH_PIN);
  
  // Convert the analog reading to a voltage
  float voltage_pH = sensorValue * (3.3 / 4096.0);
  // Convert the voltage to pH value (calibration needed for accurate results)
  // Example conversion (you'll need to calibrate this for your sensor):
  float pHValue = 21.55 - voltage_pH * 116/21;
  Serial.print("pHValue: ");
  Serial.println(pHValue,1);
  Serial.println(voltage_pH);
  return pHValue;
}

float detect_waterLevel(){
  int sensorValue = analogRead(WATER_PIN);
  float water_level = sensorValue;
  return water_level;
}

// void pause(unsigned long time1) {
// unsigned long counter1=millis();
// unsigned long interval = time1;
// while ((startTime - counter1) < interval)
// {
//   //do nothing
// }
// } //keep it for now, research other methods

void f1(float pH, float pHf, float pHc) {
  if (pH > pHc) {
    digitalWrite(PH_DOWN_PIN, HIGH);
    delay(500); // Add buffer time for pumping action
    digitalWrite(PH_DOWN_PIN, LOW);
    delay(300000);
  } else if (pH < pHf) {
    digitalWrite(PH_UP_PIN, HIGH);
    delay(500); // Add buffer time for pumping action
    digitalWrite(PH_UP_PIN, LOW);
    delay(300000); //assuming 5 mins needed for pH to settle after pump
  }
  else {
    return;
  }
}

void f2(float EC, float ECf, float ECc) {
  
  if (EC < ECf) {
    digitalWrite(NUTRIENT_PIN, HIGH);
    delay(500);
    digitalWrite(NUTRIENT_PIN, LOW);
  } else if (EC > ECc) {
    digitalWrite(flush_PIN, HIGH);
    delay(500); 
    digitalWrite(flush_PIN, LOW);
  }
  else {
    return;
  }
}

void f3(float water_level, float water_level_threshold) {
  if (water_level < water_level_threshold) { //the inequality symbol is counterintuitive as the higher the water_level, the lower its' analog value
    digitalWrite(flush_PIN, HIGH);
    delay(500);
    digitalWrite(flush_PIN, LOW);
  } else {
    digitalWrite(flush_PIN, LOW);
  }
}

void regulator(float pH, float EC, float temperature) {
  Serial.println("test1");
  if (pH < pHf || dpH > pHc)
  {
    f1(pH, pHf, pHc);
  }
  if (EC < ECf || EC > ECc){
    f2(EC, ECf, ECc);
  }
    float water_level = detect_waterLevel(); // Update water level
    f3(water_level, water_level_threshold);
    
  }


// 'setting up' 
void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
}
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  
  pinMode(TdsSensorPin,INPUT);
  pinMode(pH_PIN, INPUT);
 
  // Start the DS18B20 sensor
  sensors.begin();
  // Initialize pins
  pinMode(PH_DOWN_PIN, OUTPUT);
  pinMode(PH_UP_PIN, OUTPUT);
  pinMode(NUTRIENT_PIN, OUTPUT);
  pinMode(WATER_PIN, INPUT);
  pinMode(flush_PIN, OUTPUT);
  
  // Initialize start time
  startTime = millis();
}


void loop() {
  unsigned long currentTime = millis();
  if (currentTime - startTime >= runDuration) {// Check if the total runtime has exceeded 20 days
    Serial.println("20 days have passed. Ready to harvest!.");
    while (true); // Stop further execution
  }
  
  sensors.requestTemperatures(); 
  float temperature = sensors.getTempCByIndex(0);
  // float temperatureF = sensors.getTempFByIndex(0);
    
  Serial.print("temperature: ");
  Serial.print(temperature);
  Serial.println("ºC");

  float pH = detect_pH();
  float tdsValue = detect_TDS(temperature)/0.64;
  float EC = 0.64*tdsValue;

  // Regulate pH, EC, and water level (Add motor code into it before uncommenting this function)
  regulator(pH, EC, temperature);
  pH = detect_pH();
  float tdsValue = detect_TDS(temperature)/0.64;
  EC = 0.64*tdsValue;

if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "test/int", count)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    count++;
    
    // Write an Float number on the database path test/float
    
    
    if (Firebase.RTDB.setFloat(&fbdo, "TDS/float", detected_EC)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

if (Firebase.RTDB.setFloat(&fbdo, "pH/float", detected_pH)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "TemperatureC/float", temperature)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
}

}
