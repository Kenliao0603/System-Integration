/*
This example shows how to connect to Cayenne using an ESP32 and send/receive sample data.
The CayenneMQTT Library is required to run this sketch. If you have not already done so you can install it from the Arduino IDE Library Manager.
*/
//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP32.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#define RXD2 16
#define TXD2 17

// WiFi network info.
char ssid[] = "WINSTAR-GUEST";
char wifiPassword[] = "16527682";

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "2ed12840-d7af-11eb-a2e4-b32ea624e442";
char password[] = "5a0d54eb1f0456f6adeccd6808679e75e25d8fe1";
char clientID[] = "007d7a10-decc-11eb-b767-3f1a8f1211ba";
const unsigned long postingInterval = 20000L;
float temperature;
float amplitude;
float power;
float voltage = 110;
unsigned char r = 0;
int num;
int tick;
String txtMsg;
SoftwareSerial linkSerial(16, 17); // RX, TX

void setup() {
  Serial.begin(115200);
  while (!Serial) {};
  linkSerial.begin(4800);
	Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
}

void loop() {
	Cayenne.loop();
  while (linkSerial.available()){
    StaticJsonDocument<300> doc;
    DeserializationError err = deserializeJson(doc, linkSerial);
    if (err == DeserializationError::Ok){
      // Print the values
      // (we must use as<T>() to resolve the ambiguity)
      Serial.print("temp = ");
      temperature = doc["temp"].as<float>();
      Serial.println(temperature);

      Serial.print("amplitude = ");
      amplitude = doc["amplitude_current"].as<float>();
      Serial.println(amplitude);

      Serial.print("button = ");
      num = doc["num"].as<int>();
      Serial.println(num);

      Serial.println("---");
    }else{
      // Print error to the "debug" serial port
      Serial.print("deserializeJson() returned ");
      Serial.println(err.c_str());
  
      //Flush all bytes in the "link" serial port buffer
      while (linkSerial.available() > 0)
        linkSerial.read();
    }
    power = amplitude * voltage;
  }
}

CAYENNE_OUT_DEFAULT()
{
	// Write data to Cayenne here. This example just sends the current uptime in milliseconds on virtual channel 0.
	  Cayenne.virtualWrite(0, temperature, "temp", "c");
    Cayenne.virtualWrite(1, temperature, "temp", "c");
    Cayenne.virtualWrite(2, amplitude, "current", "a");
    Cayenne.virtualWrite(3, amplitude, "current", "a");
    Cayenne.virtualWrite(4, power, "Watts", "w");
    Cayenne.virtualWrite(6, num, "digital_sensor", "d");
}
// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN(5){
	digitalWrite(2, !getValue.asInt()); 
  //CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
	//Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}



