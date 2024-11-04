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
#define WIFI_SSID "Greenit-Arrow"
#define WIFI_PASSWORD "GreenitArrow"

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
bool regulated = false;

// "pins"
// GPIO where the DS18B20 is connected to
  
#define TdsSensorPin 36       // Define the analog pin where the pH sensor is connected
#define PH_DOWN_PIN 25
#define PH_UP_PIN 22
#define NUTRIENT_PIN 23
#define WATER_PIN 39
#define flush_PIN 32
#define pH_PIN 34 // Use any analog pin
#define oneWireBus 33
#define Relaytemp 21
#define Relayph 18
#define Relayec 19 
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
float pH = 0.00;
float temperature = 0.00;
int Rcounter = 0;



// Thresholds and desired values that will be inputed by Users
float threshold = 0.1; // Threshold for user input pH and EC
float pHf; // pH lower bound, f  for floor
float pHc; // pH upper bound, c  for ceiling
float ECf; // EC lower bound
float ECc; // EC upper bound
float expected_pH; // user desired pH value
float expected_EC; // user desired EC value
float new_expected_pH; 
float new_expected_EC; 
float water_level_threshold = 2024; // eg threshold for water level
bool received_user_input = false; // Trigger to start controlling the system

// Time constraints
unsigned long startTime;
unsigned long runDuration = 1728000000; //20 days in milliseconds
//for controlling the loop to run every 24 hours
unsigned long currentTime;
unsigned long previousTime = 0;
unsigned long runTime = 0;
const unsigned long timeframe = 36000000;  //10 hours in milliseconds 
 // Detect Tmeperature
 float detect_temperature(){  
  delay(2000);
  sensors.requestTemperatures(); 
  float temperature = sensors.getTempCByIndex(0);
  // float temperatureF = sensors.getTempFByIndex(0);
  // float temperature =10.0;
  Serial.print("temperature: ");
  Serial.print(temperature);
  Serial.println("ºC");
  return temperature;
  }
 
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
    Serial.println("TDS Raw_correct: ");
    Serial.print(analogBuffer[analogBufferIndex]);
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT)
    { 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4096.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //temperature compensation
      float compensationVoltage=averageVoltage/compensationCoefficient;
      Serial.print(compensationVoltage);
      Serial.print(averageVoltage);
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
      
      Serial.print("voltage:");
      Serial.print(averageVoltage,2);
      Serial.print("V   ");
      Serial.print("TDS Value:");
      Serial.print(tdsValue,1);
      Serial.println("ppm");
      
    }
    // Serial.print("TDS Value:");
    // Serial.print(tdsValue,1);
    // Serial.println("ppm");
  }
  return tdsValue;
}

float detect_pH(){
int sensorValue = analogRead(pH_PIN);
  
  // Convert the analog reading to a voltage
  float voltage_pH = sensorValue * (3.3 / 4096.0);
  // Convert the voltage to pH value (calibration needed for accurate results)
  // Example conversion (you'll need to calibrate this for your sensor):
  float pH = 21.55 - voltage_pH * 116/21;
  Serial.print("pHValue: ");
  Serial.println(pH,1);
  Serial.println(voltage_pH);
  return pH;
}

float detect_waterLevel(){
  int sensorValue = analogRead(WATER_PIN);
  float water_level = sensorValue;
  return water_level;
}

void f1(float pH, float pHf, float pHc) {
  if (pH > pHc) {
    digitalWrite(PH_DOWN_PIN, HIGH);
    delay(5000); // Add buffer time for pumping action
    Serial.println("Pump 1");
    digitalWrite(PH_DOWN_PIN, LOW);
    delay(3000);
  } else if (pH < pHf) {
    digitalWrite(PH_UP_PIN, HIGH);
    delay(5000); // Add buffer time for pumping action
    Serial.println("Pump 2");
    digitalWrite(PH_UP_PIN, LOW);
    delay(3000); //assuming 5 mins needed for pH to settle after pump
  }
  else {
    return;
  }
}

void f2(float EC, float ECf, float ECc) {
  
  if (EC < ECf) {
    digitalWrite(NUTRIENT_PIN, HIGH);
    delay(5000);
    Serial.println("Pump 3");
    digitalWrite(NUTRIENT_PIN, LOW);
    delay(3000);
  } else if (EC > ECc) {
    digitalWrite(flush_PIN, HIGH);
    delay(5000); 
    Serial.println("Pump 4");
    digitalWrite(flush_PIN, LOW);
    delay(3000);
  }
  else {
    return;
  }
}

void f3(float water_level, float water_level_threshold) {
  if (water_level < water_level_threshold) { //the inequality symbol is counterintuitive as the higher the water_level, the lower its' analog value
    digitalWrite(flush_PIN, HIGH);
    delay(5000);
    digitalWrite(flush_PIN, LOW);
  } else {
    digitalWrite(flush_PIN, LOW);
  }
}
void regulator(float pH, float EC, float temperature, float expected_pH, float expected_EC) {
  Serial.println("Regulating");
  pHf = (1 - threshold) * expected_pH;
  pHc = (1 + threshold) * expected_pH;
  ECf = (1 - threshold) * expected_EC;
  ECc = (1 + threshold) * expected_EC;
  // if (pH < pHf || pH > pHc)
  // {
  //   f1(pH, pHf, pHc);
  // }
  if (EC < ECf || EC > ECc){
    f2(EC, ECf, ECc);
  }
  else if (EC>=ECf && EC<=ECc && pH>=pHf && pH<=pHc) { 
    regulated = true;
  }
    float water_level = detect_waterLevel(); // Update water level
    f3(water_level, water_level_threshold);
    
  }

float check_user_input() {
  // Check if the user has inputted any values
  // If so, update the thresholds and desired values
  if (Firebase.RTDB.getFloat(&fbdo, "ExpectedPH/float")){
    expected_pH = fbdo.floatData();
    Serial.println("Expected pH: " + String(expected_pH));
  }
  else {
    Serial.println("Failed to get Expected pH");
    Serial.println("REASON: " + fbdo.errorReason());
  }
  if (Firebase.RTDB.getFloat(&fbdo, "ExpectedTDS/float")){
    expected_EC = fbdo.floatData();
    Serial.println("Expected EC: " + String(expected_EC));
  }
  else {
    Serial.println("Failed to get Expected EC");
    Serial.println("REASON: " + fbdo.errorReason());
  }
  return expected_pH, expected_EC;
}

bool check_condition() {
  digitalWrite(Relaytemp, HIGH);
  delay(10000);
  float temperature = detect_temperature();
  digitalWrite(Relaytemp, LOW);
  delay(10000);
  digitalWrite(Relayph, HIGH);
  delay(10000);
  float pH = detect_pH();
  digitalWrite(Relayph, LOW);
  delay(10000);
  if(temperature != -127.00){
      digitalWrite(Relayec, HIGH);
      Serial.println("echigh");
      delay(10000);
      for(int i=0; i < SCOUNT; i++){
        delay(1000);
        float tdsValue = detect_TDS(temperature)/0.64;
      }
      digitalWrite(Relayec, LOW);
      Serial.println("eclow");
      delay(10000);
    }
  float EC = tdsValue;
  pHf = (1 - threshold) * expected_pH;
  pHc = (1 + threshold) * expected_pH;
  ECf = (1 - threshold) * expected_EC;
  ECc = (1 + threshold) * expected_EC;

  if (EC>=ECf && EC<=ECc && pH>=pHf && pH<=pHc){
    Serial.println("No changes in EC and pH value hence returning TRUE");
       return true;
  }
  Serial.println("A change in EC and pH value hence returning FALSE");
  return false;
  }


void package(){
  while(!regulated){
    //Temp: switch on -> read -> switch off:
    digitalWrite(Relaytemp, HIGH);
    Serial.println("Temphigh");
    delay(10000);
    temperature = detect_temperature();
    digitalWrite(Relaytemp, LOW);
    Serial.println("Templow");
    delay(10000);

    //hard code temp to 22.3 celcius if -127.0
    if(temperature == -127.00){
      temperature = 22.3;
      Serial.print("Temperature overwrite to 22.3; suspect connection or sensors' issues");
    }

    //PH: switch on -> read -> switch off:
    digitalWrite(Relayph, HIGH);
    Serial.println("phhigh");
    delay(10000);
    float pH = detect_pH();
    digitalWrite(Relayph, LOW);
    Serial.println("phlow");
    delay(10000);

    //calculate EC value + switch on tds_sensor
    if(temperature != -127.00){
      digitalWrite(Relayec, HIGH);
      Serial.println("echigh");
      delay(10000);
      for(int i=0; i < SCOUNT; i++){
        delay(1000);
        float tdsValue = detect_TDS(temperature)/0.64;
      }
      digitalWrite(Relayec, LOW);
      Serial.println("eclow");
      delay(10000);
    }
      // calling regulator to adjust

      regulator(pH, tdsValue, temperature, expected_pH, expected_EC);

      //check number of times package() has run per 24hours timeframe, for debug tracing
      Rcounter+=1;
      Serial.print("Numbers of repeat in package(): ");
      Serial.println(Rcounter,1);
      update_Firebase();
  }
}

void update_Firebase(){
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
    
    
    if (Firebase.RTDB.setFloat(&fbdo, "TDS/float", tdsValue)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

if (Firebase.RTDB.setFloat(&fbdo, "pH/float", pH)){
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

  delay(5000); // delay before authenticating firebase
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
  pinMode(Relaytemp, OUTPUT);
  pinMode(Relayph, OUTPUT);
  pinMode(Relayec, OUTPUT);

  digitalWrite(Relaytemp, LOW); //set default relay pin to LOW
  digitalWrite(Relayph, LOW);
  digitalWrite(Relayec, LOW);
  
  // Initialize start time
  startTime = millis();
}


void loop() {

  while(!received_user_input) {
    expected_pH, expected_EC = check_user_input();
    if (expected_pH != 0 && expected_EC != 0) {
      received_user_input = true;
    }
  }
  currentTime = millis();
  while(currentTime - previousTime <= timeframe){
    new_expected_pH, new_expected_EC = check_user_input();
    if(new_expected_pH != expected_pH || new_expected_EC != expected_EC ){
    regulated = false;
    expected_pH = new_expected_pH;
    expected_EC = new_expected_EC;
    package();
    runTime = currentTime-previousTime;
    Serial.print("package() runtime due to user updating expected value: ");
    Serial.println(runTime);
    }
    bool unchange = check_condition();
    if(!unchange) {
      regulated = false;
      package();
    runTime = currentTime-previousTime;
    Serial.print("package() runtime(cumulative) due to user updating expected value: ");
    Serial.println(runTime);
    }
  }
  Rcounter=0;
  previousTime = currentTime;  // Update previoustime for next iteration of void loop()
  Serial.println("void loop() has run for 24 hours");
  }
  
  
