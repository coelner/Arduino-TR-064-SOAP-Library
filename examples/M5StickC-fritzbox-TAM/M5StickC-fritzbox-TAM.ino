/*
   an ESP32/8266 module  as simple ON/OFF switch for the TAM

   https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AHA-HTTP-Interface.pdf
   https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/x_tam.pdf

*/

#define BUTTON_PIN 2 //change it to the button pin
#include <tr064.h>


volatile bool toggleTAM = false;

const char* WIFI_SSID = "your SSID here";
const char* WIFI_PASS = "the wifi password";
const char* FRITZBOX_AIN = "admin"; //admin if no username is used
const char* FRITZBOX_PASS = "einsElfELF!!11";
const String FRITZBOX_DOMAIN = "fritz.box";
const int PORT = 49000;
const unsigned long UPDATE_INTERVAL = 10000UL; //Check TAM switch every 10 seconds

unsigned long lastConnection = 0;

TR064 connection(PORT, FRITZBOX_DOMAIN, FRITZBOX_AIN, FRITZBOX_PASS);

void setup() {
  pinMode(BUTTON_PIN, INPUT);
   attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPress, FALLING);

  Serial.begin( 115200 );
  if (Serial) Serial.print("ESP Board MAC Address:  ");
  if (Serial) Serial.println(WiFi.macAddress());

  if (connectWiFi()) {
    if (Serial) Serial.println( "WiFi connection established" );
    if (Serial) Serial.println("Initialize TR-064 connection");
    connection.debug_level = DEBUG_WARNING; //DEBUG_VERBOSE; //0: None, 1: Errors, 2: Warning, 3: Info, 4: Verbose
    connection.init();
  }
}

void loop() {
  yield();
  connectWiFi();
  unsigned long int now = millis();
  if ( (now - lastConnection) > UPDATE_INTERVAL ) {
    if (Serial) Serial.print("Routine TAM check: ");
    if (Serial) Serial.println(getTAMStatus() == 1 ? "ON" : "OFF");
    lastConnection = millis();
  }
  if (toggleTAM) {
    ButtonCallback();
    toggleTAM = false;
  }
}

ICACHE_RAM_ATTR void buttonPress() {
  toggleTAM = true;
}
bool connectWiFi() {
  if ( WiFi.status() == WL_CONNECTED ) {
    return true;
  }
  else {
    WiFi.mode( WIFI_STA );
    WiFi.begin( WIFI_SSID, WIFI_PASS );

    if ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
      if (Serial) Serial.println( "WiFi connection failed" );
      return false;
    }
    return true;
  }
}

int getTAMStatus() {
  String params[][2] = {"NewIndex", "0"};
  int a = 1;
  String req[][2] = {"NewEnable", "0"};
  int b = 1;
  connection.action("urn:dslforum-org:service:X_AVM-DE_TAM:1", "GetInfo", params, a, req, b);
  return req[0][1].toInt();
}

bool switchTAMStatus() {
  int currentValue = getTAMStatus();
  int newValue = (currentValue == 1) ? 0 : 1;
  String params[][2] = {{"NewIndex", "0"}, {"NewEnable", String(newValue)}};
  int a = 2;
  connection.action("urn:dslforum-org:service:X_AVM-DE_TAM:1", "SetEnable", params, a);
  return (getTAMStatus() == newValue) ? true : false;
}

void ButtonCallback() {
  if (Serial) Serial.print("Switch TAM Status: ");
  if (Serial) Serial.println((switchTAMStatus()) ? "success" : "fail");
  return;
}
