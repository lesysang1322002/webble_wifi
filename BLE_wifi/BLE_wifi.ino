#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h> //Library to use BLE as server
#include <BLE2902.h> 
WebServer server(80);
String input1,input2;
uint8_t LED1pin = 4;
bool LED1status = LOW;
uint8_t LED2pin = 5;
bool LED2status = LOW;

bool _BLEClientConnected = false;

#define BatteryService BLEUUID((uint16_t)0x181F) 
BLECharacteristic BatteryLevelCharacteristic(BLEUUID((uint16_t)0x2A18), BLECharacteristic::PROPERTY_READ  | BLECharacteristic::PROPERTY_WRITE  | BLECharacteristic::PROPERTY_NOTIFY);
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
template <typename T>
void BLE_print(T myVar) {
  String value = String(myVar);
  uint8_t data[value.length()];
  for (size_t i = 0; i < value.length(); i++) {
      data[i] = static_cast<uint8_t>(value.charAt(i));
  }
  BatteryLevelCharacteristic.setValue(data, value.length());
  BatteryLevelCharacteristic.notify();
  Serial.print(myVar);
}
void setup() {
  Serial.begin(115200);
  pinMode(2,OUTPUT);
  InitBLE();
  BLE_print("BLE\n");
  BLE_print("Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(input1,input2);
  while(WiFi.status() != WL_CONNECTED){
  Serial.print(".");
  receiData(input1,input2);
  WiFi.begin(input1,input2);
  delay(1000);
  }
  BLE_print("Connected to " + input1 +"\n");
  BLE_print("IP address: ");
  // Lấy địa chỉ IP
  IPAddress localIP = WiFi.localIP();

  // Chuyển đổi địa chỉ IP thành chuỗi
  String strIP = localIP.toString();
  BLE_print(strIP+"\n");
  server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
  server.onNotFound(handle_NotFound);
  server.begin();
  BLE_print("HTTP server started\n");
}

void loop() {
          server.handleClient();
          if(LED1status)  digitalWrite(LED1pin, HIGH);
          else            digitalWrite(LED1pin, LOW);
          if(LED2status)  digitalWrite(LED2pin, HIGH);
          else            digitalWrite(LED2pin, LOW);
        }
        void handle_OnConnect() {
          LED1status = LOW;
          LED2status = LOW;
          BLE_print("GPIO4 Status: OFF | GPIO5 Status: OFF\n");
          server.send(200, "text/html", SendHTML(LED1status,LED2status)); 
        }
        void handle_led1on() {
          LED1status = HIGH;
          BLE_print("GPIO4 Status: ON\n");
          server.send(200, "text/html", SendHTML(true,LED2status)); 
        }
        void handle_led1off() {
          LED1status = LOW;
          BLE_print("GPIO4 Status: OFF\n");
          server.send(200, "text/html", SendHTML(false,LED2status)); 
        }
        void handle_led2on() {
          LED2status = HIGH;
          BLE_print("GPIO5 Status: ON\n");
          server.send(200, "text/html", SendHTML(LED1status,true)); 
        }
        void handle_led2off() {
          LED2status = LOW;
          BLE_print("GPIO5 Status: OFF\n");
          server.send(200, "text/html", SendHTML(LED1status,false)); 
        }
        void handle_NotFound(){
          server.send(404, "text/plain", "Not found");
        }
        String SendHTML(uint8_t led1stat,uint8_t led2stat){
           String ptr = "<!DOCTYPE html> <html>\n";
           ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
           ptr +="<title>Deli Robot</title>\n";
           ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
           ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
           ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
           ptr +=".button-on {background-color: #3498db;}\n";
           ptr +=".button-on:active {background-color: #2980b9;}\n";
           ptr +=".button-off {background-color: #34495e;}\n";
           ptr +=".button-off:active {background-color: #2c3e50;}\n";
           ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
           ptr +="</style>\n";
           ptr +="</head>\n";
           ptr +="<body>\n";
           ptr +="<h1>Robot Web Server</h1>\n";
           ptr +="<h3>Deli Robot by sysang</h3>\n";
           if(led1stat)  ptr +="<p>GPIO4 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";
           else          ptr +="<p>GPIO4 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";
           if(led2stat)  ptr +="<p>GPIO5 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";
           else          ptr +="<p>GPIO5 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";
           ptr +="</body>\n";
           ptr +="</html>\n";
           return ptr;
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
  
