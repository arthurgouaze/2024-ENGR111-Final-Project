/* ----------------------------------------------------------------------------------
This baseline program serves as a starting point for your ENGR111 final project.
Please customize it by adding or modifying any necessary variables, variable values,
functions, and commands to operate the Analog/Digital Sensors and Minipump,
ensuring your system meets the performance requirements.
Be sure to review the comments carefully, as they contain helpful hints and guidance.


Stevens Institute of Technology - IDEaS Program 2024/ENGR111
-----------------------------------------------------------------------------------*/
// Group 5, Section C: Anthony, Arthur, & Juan 

#include <DHT.h> // include dht library
#include <ESP32Servo.h> // include ESP32 servo library
#include <WiFi.h> // include wifi library
#include <PubSubClient.h> // include subclient library
#include <WiFiClientSecure.h> // include wificlient library
#include "SystemLogic.h"     //This header contains certificate info for MQTT connection


//Global variable declarations or pin number settings can be done before the setup() function.
// Pin definitions
// Note! Pin number should match the actual PIN# used in your circuit
const int DHT_PIN = 32; // sets the pin# for the dht sensor      
const int PHOTOCELL_PIN = 34; // sets the pin# for the photocell
const int SOIL_PIN = 35; // sets the pin# for the soild moisture sensor
const int PUMP_PIN = 2; // sets the pin# for the minipump


// WiFi credentials
const char* WIFI_NAME = "Stevens-IoT"; // sets the wifi name
const char* WIFI_PASS = "nMN882cmg7"; // sets the wifi password


// MQTT configuration
const char* MQTT_SERVER = "98db5050a791439c98eac188febfecbe.s2.eu.hivemq.cloud"; // sets the MQTT server
const char* MQTT_USER = "stevens"; // sets the MQTT username
const char* MQTT_PASS = "Stevens@1870"; // sets the MQTT password
const int MQTT_PORT = 8883; // sets the MQTT port


// Team information
// You must follow the Topic naming protocol!
const String YEAR = "2024"; // strings the year
const String CLASS = "ENGR111"; // strings the class
const String SECTION = "C"; // Use your section name
const String GROUP_NUMBER = "N5"; // Use your group number


// Sensor configuration
// Important! The values of below two variables should be updated with the values
//            ​​measured by testing your Capacitive Soil Moisture sensor.
const int SOIL_MAXWET = 900; // sets the threshold for wet soil
const int SOIL_MAXDRY = 2600; // sets the threshold for dry soil
 
//VariableS for setting the operating range of the pump
const int PUMP_ON_ANGLE = 180;
const int PUMP_OFF_ANGLE= 90;


void setup() {
  wifi_start();
  mqtt_start();
  enable_sensors();
  Serial.begin(115200); // sets the baud rate
  // You may need additional functions in here
}


const int publishInterval = 5000;    // Sampling time


void loop() {
  mqtt_loop();


  // PUMP sample code: Start --------------------------------------------
  // The command below simply turns the pump on and off. You will need to modify the command
  // to supply a certain amount of water based on the sensor value.


  // To avoid unnecessary operation/power consumption of the pump, (attach(), detach()) was used
  // in the loop function so that the pump operates only when necessary.


  mypump.attach(PUMP_PIN, 1000, 2000);   // attaches the minipump on PUMP_PIN
  mypump.write(PUMP_ON_ANGLE);
  delay(500); //This delay contributes to open time
  mypump.write(PUMP_OFF_ANGLE);
  delay(500); //This delay contributes to close time
  mypump.detach();
  // PUMP sample code: End ----------------------------------------------
 
  int temperature = dht.readTemperature(); // reads the temperature from the dht sensor
  int temp = ((temperature * 1.8) + 32); // Converts from Celcius to Degrees Fahrenheit
  Serial.print("Temperature: "); // prints "Temperature: "
  Serial.println (temp); // prints the value for temperature

  char pubString_temp[8];     // Character array vaaiable
  // Below command Determines the format of the output data (the number of significant digits)
  dtostrf(temp, 1, 2, pubString_temp);  //dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
  String tempTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "temp";
  
  //Non-blocking MQTT publish & delay
  client.publish(tempTopic.c_str(), pubString_temp);  // publish
  delay(publishInterval);  // 100secs delay for publishing (sampling time)
 

  int airhumidity = dht.readHumidity(); // reads humidity as a %
  Serial.print ("Air Humidity: "); // prints "Air Humidity: "
  Serial.println(airhumidity); // prints the value for airhumidity


  char pubString_airhumidity[8];     // Character array vaaiable
  // Below command Determines the format of the output data (the number of significant digits)
  dtostrf(airhumidity, 1, 2, pubString_airhumidity);  //dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
  String airTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "airhumidity";
  //Non-blocking MQTT publish & delay
  client.publish(airTopic.c_str(), pubString_airhumidity);  // publish
  delay(publishInterval);  // 100secs delay for publishing (sampling time)

  // Measuring & Publishing Soil moisture sensor value: Start -----------
  int soilhumidity = analogRead(SOIL_PIN);
  // Below commands convert(map) measured values ​​to values ​​between 0 and 100%
  int soilPercent = map(soilhumidity, SOIL_MAXWET, SOIL_MAXDRY, 100, 0);
  Serial.print("Soil Humidity(analog): ");
  Serial.println(soilhumidity);
  Serial.print("Soil Humidity: "); // prints "Soil Humidity: "
  Serial.print(soilPercent); // prints the value for soildPercent
  Serial.println(" %"); // prints " %""
 
  char pubString_soilhumidity[8];     // Character array vaaiable
  // Below command Determines the format of the output data (the number of significant digits)
  dtostrf(soilPercent, 1, 2, pubString_soilhumidity);  //dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
  String soilTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "soilhumidity";
 
  //Non-blocking MQTT publish & delay
  client.publish(soilTopic.c_str(), pubString_soilhumidity);  // publish
  delay(publishInterval);  // 100secs delay for publishing (sampling time)
  // Measuring & Publishing Soil moisture sensor value: END -----------
  }


