#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#include <cppQueue.h>

#include <SPI.h>
#include <can-serial.h>
#include <mcp2515_can.h>
#include <mcp2515_can_dfs.h>
#include <mcp2518fd_can.h>
#include <mcp2518fd_can_dfs.h>
#include <mcp_can.h>
#include <math.h>

#define pinTempSensor A0
#define ELECTRICITY_SENSOR SCL
#define CAN_2515
#define	MAX_LENGTH		100

// Set SPI CS Pin according to your hardware

#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
// For Wio Terminal w/ MCP2518FD RPi Hat：
// Channel 0 SPI_CS Pin: BCM 8
// Channel 1 SPI_CS Pin: BCM 7
// Interupt Pin: BCM25
const int SPI_CS_PIN  = BCM8;
const int CAN_INT_PIN = BCM25;
#else

// For Arduino MCP2515 Hat:
// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
#endif

#ifdef CAN_2518FD
#include "mcp2518fd_can.h"
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

#ifdef CAN_2515
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif
//char mystr[10] = "Hello\n"; //String data
const unsigned long postingInterval = 200L;
float temp;
float lasttemp;
unsigned long lastConnectionTime = 0;
long lastUpdateTime = 0; 
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
float lastamp;
float amplitude_current;
float val;
int num;
unsigned char digit[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char bar[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char len = 0;
typedef struct {
  
  unsigned long id;
  unsigned char toggle[8];
  
}CanBusPacket_t;

CanBusPacket_t canBusPacket;
CanBusPacket_t canBusPacketTest;
unsigned char cnt = 0;
cppQueue	q(sizeof(CanBusPacket_t), MAX_LENGTH, FIFO, true);
SoftwareSerial linkSerial(6, 7); // RX, TX

void setup() {
// Begin the Serial at 9600 Baud
  Serial.begin(9600);
  while(!Serial) {};
  linkSerial.begin(4800);
  pinMode(pinTempSensor, INPUT);
  pinMode(ELECTRICITY_SENSOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING); // start interrupt
  while (CAN_OK != CAN.begin(CAN_250KBPS)) {             // init can bus : baudrate = 500k
      SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
      delay(100);
  }
  SERIAL_PORT_MONITOR.println("CAN init ok!");
  CAN.init_Mask(0, 0, 0x7ff);                         // there are 2 mask in mcp2515, you need to set both of them
  CAN.init_Mask(1, 0, 0x7ff);
  CAN.init_Filt(0, 0, 0x601);                         // there are 6 filter in mcp2515
}


void loop() {
  int a;
  float R;
  if (millis() - lastUpdateTime >=  postingInterval) {        
    a = analogRead(pinTempSensor);
    R = 1023.0/a-1.0;
    R = R0*R;
    temp = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
    
    int sensor_max;
    sensor_max = getMaxValue();
    //the VCC on the Grove interface of the sensor is 5v
    amplitude_current = (float)sensor_max/1024*5/800*2000000;

    if (!q.isEmpty()){               // Only if q is not empty
       q.pop(&canBusPacketTest);    

      if(canBusPacketTest.toggle[4] == 0x01 && 
        canBusPacketTest.toggle[0] == 0x2B && 
        canBusPacketTest.toggle[1]==0x00 && 
        canBusPacketTest.toggle[2]==0x20 && 
        canBusPacketTest.toggle[3]==0x08){
        num = 1;               
      }else if(canBusPacketTest.toggle[4] == 0x00 && 
        canBusPacketTest.toggle[0] == 0x2B && 
        canBusPacketTest.toggle[1]==0x00 && 
        canBusPacketTest.toggle[2]==0x20 && 
        canBusPacketTest.toggle[3]==0x08){
        num = 0;
      }
    }

    Serial.print("temperature = ");
    Serial.println(temp);
    Serial.print("current = ");
    Serial.println(amplitude_current);
    Serial.print("button = ");
    Serial.println(num);
    Serial.println("---");

    StaticJsonDocument<200> doc;
    doc["temp"] = temp;
    doc["amplitude_current"] = amplitude_current;
    doc["num"] = num;

    serializeJson(doc, linkSerial);

    lastUpdateTime = millis();
  }
  
  int amp = amplitude_current/10;
  unsigned char digit[8] = {0x2B, 0x02, 0x20, 0x07, amp, 0x00, 0x00, 0x00};
  unsigned char bar[8] = {0x2B, 0x03, 0x20, 0x07, temp, 0x00, 0x00, 0x00};
  
  if(lastamp != amp){
    CAN.sendMsgBuf(0x67B, 0, 8, digit);
    lastamp = amp;
  }
  if(lasttemp != temp){
    CAN.sendMsgBuf(0x67B, 0, 8, bar);
    lasttemp = temp;
  }
}


void MCP2515_ISR(){
  if (CAN_MSGAVAIL == CAN.checkReceive()){ 
    unsigned char toggle[8];
    CAN.readMsgBuf(&len, toggle);
    canBusPacket.id = CAN.getCanId();
    canBusPacket.toggle[0] = toggle[0];
    canBusPacket.toggle[1] = toggle[1];
    canBusPacket.toggle[2] = toggle[2];
    canBusPacket.toggle[3] = toggle[3];
    canBusPacket.toggle[4] = toggle[4];
    canBusPacket.toggle[5] = toggle[5];
    canBusPacket.toggle[6] = toggle[6];
    canBusPacket.toggle[7] = toggle[7];       
    q.push(&canBusPacket);          // Push char to cppQueue
  }
}

int getMaxValue(){
  int sensorValue;             //value read from the sensor
  int sensorMax = 0;
  uint32_t start_time = millis();
  while((millis()-start_time) < 1000){//sample for 1000ms
    sensorValue = analogRead(ELECTRICITY_SENSOR);
    if (sensorValue > sensorMax){
      /*record the maximum sensor value*/
      sensorMax = sensorValue;
    }
  }
  return sensorMax;
}

