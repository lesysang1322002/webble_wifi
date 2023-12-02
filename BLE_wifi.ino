#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h> //Library to use BLE as server
#include <BLE2902.h> 

String input1,input2;

bool _BLEClientConnected = false;

#define BatteryService BLEUUID((uint16_t)0x180F) 
BLECharacteristic BatteryLevelCharacteristic(BLEUUID((uint16_t)0x2A19), BLECharacteristic::PROPERTY_READ  | BLECharacteristic::PROPERTY_WRITE  | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor BatteryLevelDescriptor(BLEUUID((uint16_t)0x2901));

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      _BLEClientConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      _BLEClientConnected = false;
    }
};


/* ###############################################################  CALL back to receive data from Phone */
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

class MyCallbacks: public BLECharacteristicCallbacks {

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      Serial.println(rxValue[0]);
 
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
 
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();
        Serial.println("*********");
      }
 
    }
};
/* ############################################################### */

void InitBLE() {
  BLEDevice::init("ConfigWifi");
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pBattery = pServer->createService(BatteryService);

  pBattery->addCharacteristic(&BatteryLevelCharacteristic);
  BatteryLevelDescriptor.setValue("Percentage 0 - 100");
  BatteryLevelCharacteristic.addDescriptor(&BatteryLevelDescriptor);
  BatteryLevelCharacteristic.addDescriptor(new BLE2902());


  /* ###############################################################  define callback */
  BLECharacteristic *pWriteCharacteristic = pBattery->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
 
  pWriteCharacteristic->setCallbacks(new MyCallbacks());
  /* ############################################################### */
  
  pServer->getAdvertising()->addServiceUUID(BatteryService);

  pBattery->start();
  // Start advertising
  pServer->getAdvertising()->start();
}
void setup() {
  Serial.begin(115200);
  pinMode(13,OUTPUT);
  InitBLE();
  Serial.println("BLE");
  Serial.print("Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(input1,input2);
  while(WiFi.status() != WL_CONNECTED){
  receiData(input1,input2);
  WiFi.begin(input1,input2);
  delay(1000);
  Serial.print(".");
  }
  Serial.println("Connected to " + input1 );
}

void loop() {
 digitalWrite(13,HIGH);
 delay(500);
 digitalWrite(13,LOW);
 delay(500);
}
void receiData(String& input1, String& input2){
  std::string data = BatteryLevelCharacteristic.getValue();
  String receivedData = "";
  for (int i = 0; i < data.length(); i++) {
    receivedData += char(data[i]);
  }
  size_t delimiterPos = receivedData.indexOf(';');

  if (delimiterPos != -1) {
    input1 = receivedData.substring(0, delimiterPos);
    input2 = receivedData.substring(delimiterPos + 1);
  } 
  else {
    input1 = receivedData;
    input2 = "";
    }
  }
