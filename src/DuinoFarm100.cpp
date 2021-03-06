/*
 ______   __   __  ___   __    _  _______  _______  _______  ______    __   __ 
|      | |  | |  ||   | |  |  | ||       ||       ||   _   ||    _ |  |  |_|  |
|  _    ||  | |  ||   | |   |_| ||   _   ||    ___||  |_|  ||   | ||  |       |
| | |   ||  |_|  ||   | |       ||  | |  ||   |___ |       ||   |_||_ |       |
| |_|   ||       ||   | |  _    ||  |_|  ||    ___||       ||    __  ||       |
|       ||       ||   | | | |   ||       ||   |    |   _   ||   |  | || ||_|| |
|______| |_______||___| |_|  |__||_______||___|    |__| |__||___|  |_||_|   |_|

 Name:     DuinoFarm 
 Date:     May 2022
 Author:   Flavio L Puhl Jr <flavio_puhl@hotmail.com> 
 GIT:      
 About:    DuinoCoin + MQTT publsih + DuinoCoin API get
 
Update comments                                      
+-----------------------------------------------------+------------------+---------------+
|               Feature added                         |     Version      |      Date     |
+-----------------------------------------------------+------------------+---------------+
| Initial Release based Official ESP8266 Miner 3.18   |      1.0.0       |     MAY/22    |
|                                                     |                  |               |
|                                                     |                  |               |
+-----------------------------------------------------+------------------+---------------+


Library versions                                       
+-----------------------------------------+------------------+-------------------------- +
|       Library                           |     Version      |          Creator          |
+-----------------------------------------+------------------+-------------------------- +
| PubSubClient                            |      @^2.8       |        knolleary          |
|	ArduinoJson                             |      @^6.18.5    |        bblanchon          |
|	NTPClient                               |      @^3.1.0     |        arduino-libraries  |  
+-----------------------------------------+------------------+-------------------------- +


Upload settings 
+----------------------------------------------------------------------------------------+
| PLATFORM: Espressif 8266 (3.2.0) > NodeMCU 1.0 (ESP-12E Module)                        |
| HARDWARE: ESP8266 160MHz, 80KB RAM, 4MB Flash                                          |
| PACKAGES:                                                                              |
|  - framework-arduinoespressif8266 3.30002.0 (3.0.2)                                    |
|  - tool-esptool 1.413.0 (4.13)                                                         |
|  - tool-esptoolpy 1.30000.201119 (3.0.0)                                               |
|  - toolchain-xtensa 2.100300.210717 (10.3.0)                                           |
|                                                                                        |
| RAM:   [====      ]  39.4% (used 32464 bytes from 81920 bytes)                         |
| Flash: [=====     ]  45.1% (used 472044 bytes from 1044464 bytes)                      |
+----------------------------------------------------------------------------------------+

Notes:

  EN pin ??? The ESP8266 chip is enabled when EN pin is pulled HIGH. When pulled LOW the chip works at minimum power.

  GPIO Pin - Each digital enabled GPIO can be configured to internal pull-up or pull-down, or set to high impedance. 
*/

/*+--------------------------------------------------------------------------------------+
 *| Libraries                                                                            |
 *+--------------------------------------------------------------------------------------+ */
/* If optimizations cause problems, change them to -O0 (the default)
  NOTE: For even better optimizations also edit your Crypto.h file.
  On linux that file can be found in the following location:
  ~/.arduino15//packages/esp8266/hardware/esp8266/3.0.2/cores/esp8266/ */
#pragma GCC optimize ("-Ofast")

/* If during compilation the line below causes a
  "fatal error: arduinoJson.h: No such file or directory"
  message to occur; it means that you do NOT have the
  ArduinoJSON library installed. To install it, 
  go to the below link and follow the instructions: 
  https://github.com/revoxhere/duino-coin/issues/832 */
#include <ArduinoJson.h>

/* If during compilation the line below causes a
  "fatal error: Crypto.h: No such file or directory"
  message to occur; it means that you do NOT have the
  latest version of the ESP8266/Arduino Core library.
  To install/upgrade it, go to the below link and
  follow the instructions of the readme file:
  https://github.com/esp8266/Arduino */
#include <bearssl/bearssl.h>
#include <TypeConversion.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <ESP8266WebServer.h>

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <ESP8266HTTPClient.h>                         // API access

// Uncomment the line below if you wish to register for IOT updates with an MQTT broker
#define USE_MQTT

/*+--------------------------------------------------------------------------------------+
 *| Constants declaration                                                                |
 *+--------------------------------------------------------------------------------------+ */

#ifdef USE_MQTT
  const char *ID = "DuinoFarmDev";                        // Name of our device, must be unique
  const char *TOPIC = "Duinofarm/data";                   // Topic to subcribe to
  //const char* BROKER_MQTT = "mqtt.eclipseprojects.io";  // MQTT Cloud Broker URL
  const char* BROKER_MQTT = "broker.hivemq.com";

  String swversion = __FILE__;

  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP);

  WiFiClient wclient;
  PubSubClient client(wclient);                         // Setup MQTT client
#endif

/*+--------------------------------------------------------------------------------------+
 *| Global Variables                                                                     |
 *+--------------------------------------------------------------------------------------+ */

 float uptime = 0;
 double result_balance_balance = 0;
 int total_miners = 0;
 double result_total_hashrate = 0;
 unsigned long loop1 = 0;                             // stores the value of millis() in each iteration of loop()

// Change the part in brackets to your WiFi name
const char* SSID = "CasaDoTheodoro1";
// Change the part in brackets to your WiFi password
const char* PASSWORD = "09012011";
// Change the part in brackets to your Duino-Coin username
const char* USERNAME = "flaviopuhl";
// Change the part in brackets if you want to set a custom miner name (use Auto to autogenerate, None for no name)
const char* RIG_IDENTIFIER = "duinofarmmaster";
// Change the part in brackets to your mining key (if you enabled it in the wallet)
const char* MINER_KEY = "None";
// Change false to true if using 160 MHz clock mode to not get the first share rejected
const bool USE_HIGHER_DIFF = true;
// Change true to false if you don't want to host the dashboard page
const bool WEB_DASHBOARD = false;
// Change false to true if you want to update hashrate in browser without reloading page
const bool WEB_HASH_UPDATER = false;
// Change true to false if you want to disable led blinking(But the LED will work in the beginning until esp connects to the pool)
const bool LED_BLINKING = true;

/* Do not change the lines below. These lines are static and dynamic variables
   that will be used by the program for counters and measurements. */
const char * DEVICE = "ESP8266";
const char * POOLPICKER_URL[] = {"https://server.duinocoin.com/getPool"};
const char * MINER_BANNER = "Official ESP8266 Miner";
const char * MINER_VER = "3.18";
unsigned int share_count = 0;
unsigned int port = 0;
unsigned int difficulty = 0;
float hashrate = 0;
String AutoRigName = "";
String host = "";
String node_id = "";

/*+--------------------------------------------------------------------------------------+
 *| Duino Coin Methods                                                                   |
 *+--------------------------------------------------------------------------------------+ */

void UpdateHostPort(String input) {
  // Thanks @ricaun for the code
  DynamicJsonDocument doc(256);
  deserializeJson(doc, input);
  const char* name = doc["name"];
  
  host = String((const char*)doc["ip"]);
  port = int(doc["port"]);
  node_id = String(name);

  Serial.println("Poolpicker selected the best mining node: " + node_id);
}

String httpGetString(String URL) {
  String payload = "";
  WiFiClientSecure dcclient;
  dcclient.setInsecure();
  HTTPClient http;
  
  if (http.begin(dcclient, URL)) {
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) payload = http.getString();
    else Serial.printf("Error fetching node from poolpicker: %s\n", http.errorToString(httpCode).c_str());

    http.end();
  }
  return payload;
}

void UpdatePool() {
  String input = "";
  int waitTime = 1;
  int poolIndex = 0;
  int poolSize = sizeof(POOLPICKER_URL) / sizeof(char*);

  while (input == "") {
    Serial.println("Fetching mining node from the poolpicker in " + String(waitTime) + "s");
    input = httpGetString(POOLPICKER_URL[poolIndex]);
    poolIndex += 1;

    // Check if pool index needs to roll over
    if( poolIndex >= poolSize ){
      poolIndex %= poolSize;
      delay(waitTime * 1000);

      // Increase wait time till a maximum of 32 seconds (addresses: Limit connection requests on failure in ESP boards #1041)
      waitTime *= 2;
      if( waitTime > 32 )
        waitTime = 32;
    }
  }

  // Setup pool with new input
  UpdateHostPort(input);
}

WiFiClient dcclient;
String client_buffer = "";
String chipID = "";
String START_DIFF = "";

// Loop WDT... please don't feed me...
// See lwdtcb() and lwdtFeed() below
Ticker lwdTimer;
#define LWD_TIMEOUT   60000

unsigned long lwdCurrentMillis = 0;
unsigned long lwdTimeOutMillis = LWD_TIMEOUT;

#define END_TOKEN  '\n'
#define SEP_TOKEN  ','

#define LED_BUILTIN 2

#define BLINK_SHARE_FOUND    1
#define BLINK_SETUP_COMPLETE 2
#define BLINK_CLIENT_CONNECT 3
#define BLINK_RESET_DEVICE   5

/*+--------------------------------------------------------------------------------------+
 *| Connect to WiFi network                                                              |
 *+--------------------------------------------------------------------------------------+ */

void SetupWifi() {
  Serial.println("Connecting to: " + String(SSID));
  WiFi.mode(WIFI_STA); // Setup ESP in client mode
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(SSID, PASSWORD);

  int wait_passes = 0;
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++wait_passes >= 10) {
      WiFi.begin(SSID, PASSWORD);
      wait_passes = 0;
    }
  }

  Serial.println("Successfully connected to WiFi");
  Serial.println("Local IP address: " + WiFi.localIP().toString());
  Serial.println("Rig name: " + String(RIG_IDENTIFIER));
  Serial.println();

  UpdatePool();
}

/*+--------------------------------------------------------------------------------------+
 *| OTA                                                                                  |
 *+--------------------------------------------------------------------------------------+ */

void SetupOTA() {
  // Prepare OTA handler
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.setHostname(RIG_IDENTIFIER); // Give port a name not just address
  ArduinoOTA.begin();
}

/*+--------------------------------------------------------------------------------------+
 *| Blink Method                                                                         |
 *+--------------------------------------------------------------------------------------+ */

void blink(uint8_t count, uint8_t pin = LED_BUILTIN) {
  if (LED_BLINKING){
    uint8_t state = HIGH;

    for (int x = 0; x < (count << 1); ++x) {
      digitalWrite(pin, state ^= HIGH);
      delay(50);
    }
  }
}

void RestartESP(String msg) {
  Serial.println(msg);
  Serial.println("Restarting ESP...");
  blink(BLINK_RESET_DEVICE);
  ESP.reset();
}

// Our new WDT to help prevent freezes
// code concept taken from https://sigmdel.ca/michel/program/esp8266/arduino/watchdogs2_en.html
//void ICACHE_RAM_ATTR lwdtcb(void) {
void IRAM_ATTR lwdtcb(void) {  
  if ((millis() - lwdCurrentMillis > LWD_TIMEOUT) || (lwdTimeOutMillis - lwdCurrentMillis != LWD_TIMEOUT))
    RestartESP("Loop WDT Failed!");
}

void lwdtFeed(void) {
  lwdCurrentMillis = millis();
  lwdTimeOutMillis = lwdCurrentMillis + LWD_TIMEOUT;
}

void VerifyWifi() {
  while (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0))
    WiFi.reconnect();
}

void handleSystemEvents(void) {
  VerifyWifi();
  ArduinoOTA.handle();
  yield();
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int max_index = data.length() - 1;

  for (int i = 0; i <= max_index && found <= index; i++) {
    if (data.charAt(i) == separator || i == max_index) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == max_index) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void waitForClientData(void) {
  client_buffer = "";

  while (dcclient.connected()) {
    if (dcclient.available()) {
      client_buffer = dcclient.readStringUntil(END_TOKEN);
      if (client_buffer.length() == 1 && client_buffer[0] == END_TOKEN)
        client_buffer = "???\n"; // NOTE: Should never happen

      break;
    }
    handleSystemEvents();
  }
}

void ConnectToServer() {
  if (dcclient.connected())
    return;

  Serial.println("\n\nConnecting to the Duino-Coin server...");
  while (!dcclient.connect(host, port));

  waitForClientData();
  Serial.println("Connected to the server. Server version: " + client_buffer );
  blink(BLINK_CLIENT_CONNECT); // Sucessfull connection with the server
}

bool max_micros_elapsed(unsigned long current, unsigned long max_elapsed) {
  static unsigned long _start = 0;

  if ((current - _start) > max_elapsed) {
    _start = current;
    return true;
  }
  return false;
}


#ifdef USE_MQTT

/*+--------------------------------------------------------------------------------------+
 *| Reconnect to MQTT client                                                             |
 *+--------------------------------------------------------------------------------------+ */
 
void reconnect() {
   while (!client.connected()) {                       /* Loop until we're reconnected */
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID)) {
      Serial.println("connected");
      Serial.print("ID:  "); Serial.println(ID);
      Serial.print("Publishing to: ");
      Serial.println(TOPIC);
      Serial.println('\n');

    } else {
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      SetupWifi();
    }
  }
}

/*+--------------------------------------------------------------------------------------+
 *| Get Duino Coin API data                                                              |
 *+--------------------------------------------------------------------------------------+ */

void duinocoinapi(){

WiFiClientSecure wificlient;                                    // Works for Plataformio
wificlient.setInsecure();                                       //https://maakbaas.com/esp8266-iot-framework/logs/https-requests/
HTTPClient http;

http.begin(wificlient, "https://server.duinocoin.com//users/" + String(USERNAME));
int httpCode = http.GET();                                    // Send the request
 
  if (httpCode > 0) 
  {                                         // Check the returning code
   String Duinopayload = http.getString();                    // Get the request response payload
      //Serial.println("input");                              // for debug only
      //Serial.println(Duinopayload);

  DynamicJsonDocument docdoc(6144);                           // Code from Arduino JSON assistant ( https://arduinojson.org/v6/assistant/ )

  DeserializationError error = deserializeJson(docdoc, Duinopayload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
     return;
    }
  
JsonObject result = docdoc["result"];

JsonObject result_balance = result["balance"];
result_balance_balance = result_balance["balance"]; // 8.633020799360171

total_miners = 0;
result_total_hashrate = 0;
    for (JsonObject result_miner : result["miners"].as<JsonArray>()) 
    {

      double result_miner_hashrate = result_miner["hashrate"]; // 11196.59, 80501, 78379.33333333333
      result_total_hashrate = result_total_hashrate + result_miner_hashrate;
      
      total_miners++;

    }

  }

}


/*+--------------------------------------------------------------------------------------+
 *| Get Date & Time                                                                      |
 *+--------------------------------------------------------------------------------------+ */
 
String DateAndTime(){

    timeClient.setTimeOffset(-10800);                       // Set offset time in seconds to adjust for your timezone, for example:
                                                            // GMT +1 = 3600
                                                            // GMT +8 = 28800
                                                            // GMT -1 = -3600
                                                            // GMT 0 = 0
    //while(!timeClient.update()) {
      timeClient.forceUpdate();
    //}

  time_t epochTime = timeClient.getEpochTime();              // The time_t type is just an integer. 
                                                             // It is the number of seconds since the Epoch.
  struct tm * tm = localtime(&epochTime);
  char dts[22];
    strftime(dts, sizeof(dts), "%d%b%Y %H-%M-%S", tm);       // https://www.cplusplus.com/reference/ctime/strftime/
  
  return dts;
 
}

/*+--------------------------------------------------------------------------------------+
 *| Serialize JSON and publish MQTT                                                      |
 *+--------------------------------------------------------------------------------------+ */

void SerializeAndPublish() {

  if (client.connected())                            /* Reconnect if connection to MQTT is lost */
  { Serial.println("MQTT client is connected"); }
  else
  { Serial.println("MQTT client is not connected");
  reconnect(); }

  if(client.loop())                                   /* called regularly to allow the client to process incoming messages and maintain its connection to the server */
  { Serial.println("MQTT client is still connected"); }
  else
  { Serial.println("MQTT client is no longer connected"); }

  char buff[20];                                      /* Buffer to allocate decimal to string conversion */
  char buffer[256];                                   /* JSON serialization */
  
  
    StaticJsonDocument<256> doc;                         /* See ArduinoJson Assistant V6 */
    
      doc["Device"] = String(RIG_IDENTIFIER);
      doc["Version"] = swversion;
      doc["RSSI (db)"] = WiFi.RSSI();
      doc["IP"] = WiFi.localIP();
      doc["LastRoll"] = DateAndTime();
      doc["UpTime (h)"] = uptime;
      doc["Total balance"] = dtostrf(result_balance_balance, 15, 6, buff);
      doc["Total miners"] = dtostrf(total_miners, 2, 0, buff);
      doc["Total hashrate (kH/s)"] = dtostrf(result_total_hashrate/1000, 4, 1, buff);
    
    
    serializeJson(doc, buffer);
      Serial.println("JSON Payload:");
    serializeJsonPretty(doc, Serial);                 /* Print JSON payload on Serial port */        
      Serial.println("");
                         
      Serial.print("Sending message to MQTT topic ... ");
    //client.publish(TOPIC, buffer);                    /* Publish data to MQTT Broker */
      if (client.publish(TOPIC, buffer))                           
      { Serial.println("publish succeeded"); }
      else
      { Serial.println("publish failed, either connection lost or message too large"); }
      
      Serial.println("");

}

#endif

/*+--------------------------------------------------------------------------------------+
 *| Setup                                                                                |
 *+--------------------------------------------------------------------------------------+ */

void setup() {
  Serial.begin(115200);
  Serial.println("\nDuino-Coin " + String(MINER_VER));
  pinMode(LED_BUILTIN, OUTPUT);

 
  // Autogenerate ID if required
  chipID = String(ESP.getChipId(), HEX);
  
  if(strcmp(RIG_IDENTIFIER, "Auto") == 0 ){
    AutoRigName = "ESP8266-" + chipID;
    AutoRigName.toUpperCase();
    RIG_IDENTIFIER = AutoRigName.c_str();
  }

  SetupWifi();
  SetupOTA();

  lwdtFeed();
  lwdTimer.attach_ms(LWD_TIMEOUT, lwdtcb);
  if (USE_HIGHER_DIFF) START_DIFF = "ESP8266H";
  else START_DIFF = "ESP8266";

  blink(BLINK_SETUP_COMPLETE);

  #ifdef USE_MQTT

    Serial.println("MQTT enabled...");

    swversion = (swversion.substring((swversion.indexOf(".")), (swversion.lastIndexOf("\\")) + 1))+" "+__DATE__+" "+__TIME__;   
    Serial.println("SW version: " + swversion);

    Serial.println("Broker MQTT setting server.. ");	
    client.setBufferSize (512);
    client.setServer(BROKER_MQTT, 1883);                /* MQTT port, unsecure */

    Serial.println("Starting timeclient server.. "); 	
    timeClient.begin();                                 /* Initialize a NTPClient to get time */

    Serial.println("Duino API data request... ");  
    duinocoinapi();
    
    Serial.print("Initial MQTT publish .. "); 
    SerializeAndPublish();  
  #endif

}

/*+--------------------------------------------------------------------------------------+
 *| Main Loop                                                                            |
 *+--------------------------------------------------------------------------------------+ */

void loop() {
  br_sha1_context sha1_ctx, sha1_ctx_base;
  uint8_t hashArray[20];
  String duco_numeric_result_str;

  unsigned long currentMillis = millis();             // capture the latest value of millis()
  uptime = millis()/3600000;                          // Update uptime 
  
  // 1 minute watchdog
  lwdtFeed();

  #ifdef USE_MQTT
    /*------MQTT loop 10 min ------*/
    
    if (currentMillis - loop1 >= 1*60*1000) {        

    Serial.println("Duino API data request... ");  
      duinocoinapi();
    
    Serial.print("MQTT publish ... "); 
      SerializeAndPublish(); 
      loop1 = currentMillis;
    } 
  #endif

  // OTA handlers
  VerifyWifi();
  ArduinoOTA.handle();
  
  ConnectToServer();
  Serial.println("Asking for a new job for user: " + String(USERNAME));

  dcclient.print("JOB," + 
              String(USERNAME) + SEP_TOKEN +
              String(START_DIFF) + SEP_TOKEN +
              String(MINER_KEY) + END_TOKEN);
  

  waitForClientData();
  String last_block_hash = getValue(client_buffer, SEP_TOKEN, 0);
  String expected_hash = getValue(client_buffer, SEP_TOKEN, 1);
  difficulty = getValue(client_buffer, SEP_TOKEN, 2).toInt() * 100 + 1;

  int job_len = last_block_hash.length() + expected_hash.length() + String(difficulty).length();
  Serial.println("Received job with size of " + String(job_len) + " bytes");
  expected_hash.toUpperCase();
  br_sha1_init(&sha1_ctx_base);
  br_sha1_update(&sha1_ctx_base, last_block_hash.c_str(), last_block_hash.length());

  float start_time = micros();
  max_micros_elapsed(start_time, 0);

  String result = "";
  digitalWrite(LED_BUILTIN, HIGH);
  for (unsigned int duco_numeric_result = 0; duco_numeric_result < difficulty; duco_numeric_result++) {
    // Difficulty loop
    sha1_ctx = sha1_ctx_base;
    duco_numeric_result_str = String(duco_numeric_result);
    br_sha1_update(&sha1_ctx, duco_numeric_result_str.c_str(), duco_numeric_result_str.length());
    br_sha1_out(&sha1_ctx, hashArray);
    result = experimental::TypeConversion::uint8ArrayToHexString(hashArray, 20);
    if (result == expected_hash) {
      // If result is found
      unsigned long elapsed_time = micros() - start_time;
      float elapsed_time_s = elapsed_time * .000001f;
      hashrate = duco_numeric_result / elapsed_time_s;
      share_count++;
      blink(BLINK_SHARE_FOUND);
      dcclient.print(String(duco_numeric_result)
                   + ","
                   + String(hashrate)
                   + ","
                   + String(MINER_BANNER)
                   + " "
                   + String(MINER_VER)
                   + ","
                   + String(RIG_IDENTIFIER)
                   + ",DUCOID"
                   + String(chipID)
                   + "\n");

      waitForClientData();
      Serial.println(client_buffer
                     + " share #"
                     + String(share_count)
                     + " (" + String(duco_numeric_result) + ")"
                     + " hashrate: "
                     + String(hashrate / 1000, 2)
                     + " kH/s ("
                     + String(elapsed_time_s)
                     + "s)");
      break;
    }
    if (max_micros_elapsed(micros(), 500000)) {
      handleSystemEvents();
    }
    else {
      delay(0);
    }
  } 

}