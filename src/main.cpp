#include <AsyncElegantOTA.h>
/****************************************************************************************************************************
  Async_AutoConnect_ESP8266_minimal.ino
  For ESP8266 / ESP32 boards
  Built by Khoi Hoang https://github.com/khoih-prog/ESPAsync_WiFiManager
  Licensed under MIT license
 *****************************************************************************************************************************/
#if !(defined(ESP8266))
#error This code is intended to run on ESP8266 platform! Please check your Tools->Board setting.
#endif
#include <ESPAsync_WiFiManager.h> //https://github.com/khoih-prog/ESPAsync_WiFiManager
AsyncWebServer webServer(80);
AsyncDNSServer dnsServer;
ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "My-ESP8266");

//Customized home page
String myHostName = "My-ESP8266";
String myESP8266page = "<a href='/update'>Update Firmware</span></a>";
String myNotFoundPage = "<h2>Error, page not found! <a href='/'>Go back to main page!</a></h2>";
 
//ESP8266 Access credential
const char *myUsername = "admin";
const char *myPass = "admin";


//TCP server client 
const char* host = "10.10.1.3";
const uint16_t port = 7878;
const int relePin = 4;
const unsigned int triggerTime = 250;
WiFiClient client;

void connectToServer();

void setup()
{
    //Forece rele state LOW
    pinMode(relePin, OUTPUT);
    digitalWrite(relePin,LOW);

    Serial.begin(115200);
    while (!Serial);
    delay(200);
    Serial.print("\nStarting Async_AutoConnect_ESP8266_minimal on " + String(ARDUINO_BOARD));
    Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);
    
    //ESPAsync_wifiManager.resetSettings();   //reset saved settings
    //ESPAsync_wifiManager.setAPStaticIPConfig(IPAddress(192,168,186,1), IPAddress(192,168,186,1), IPAddress(255,255,255,0));
    Serial.println("Connect to previously saved AP...");
    ESPAsync_wifiManager.autoConnect("My-ESP8266");
 
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print(F("Connected. Local IP: "));
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));
        Serial.println("Can't connect! Enter WiFi config mode...");
        Serial.println("Restart...");
        ESP.reset();
    }
 
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                 { request->send(200, "text/html", myESP8266page); });
    webServer.onNotFound([](AsyncWebServerRequest *request)
                         { request->send(404, "text/html", myNotFoundPage); });
 
    AsyncElegantOTA.begin(&webServer, myUsername, myPass); // Start ElegantOTA
    webServer.begin();
    Serial.println("FOTA server ready!");
}
 
void loop()
{
 // Check if data is available from the server
  if (client.available()) {
    String data = client.readStringUntil('\n');
    
    Serial.print("Received data: ");
    Serial.println(data);
    if(data == "open"){
      digitalWrite(relePin, HIGH);
      
      delay(triggerTime);

      digitalWrite(relePin,LOW);
      client.write("ok!\n");

      delay(triggerTime * 4);
      
      while (client.available()) {
        client.read();
      }

    }
    if(data == "reset"){
      ESPAsync_wifiManager.resetSettings();
      ESP.eraseConfig(); 
      delay(2000);
      ESP.reset(); 
    }
  }

  // Check if the server connection is lost and attempt reconnection
  if (!client.connected()) {
    Serial.println("Connection lost. Reconnecting...");
    connectToServer();
  }
}

void connectToServer() {
  while (!client.connect(host, port)) {
    Serial.println("Failed to connect to server. Retrying in 5 seconds...");
    delay(5000);
  }
  Serial.println("Connected to server");
  client.write("Hello from esp8266!\n");
}