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
 About:    DuinoCoin + MQTT publsih powered by solar panel 
 
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
| RAM:   [====      ]  37.6% (used 30796 bytes from 81920 bytes)                         |
| Flash: [====      ]  44.2% (used 461641 bytes from 1044464 bytes)                      |
+----------------------------------------------------------------------------------------+

Notes:

  EN pin – The ESP8266 chip is enabled when EN pin is pulled HIGH. When pulled LOW the chip works at minimum power.

  GPIO Pin - Each digital enabled GPIO can be configured to internal pull-up or pull-down, or set to high impedance. 
*/

/*+--------------------------------------------------------------------------------------+
 *| Libraries                                                                            |
 *+--------------------------------------------------------------------------------------+ */
// Libraries built into IDE
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
#include <NTPClient.h>

// Uncomment the line below if you wish to register for IOT updates with an MQTT broker
// #define USE_MQTT

// Uncomment the line below if you wish to use a DHT sensor (Duino IoT beta)
// #define USE_DHT

/*+--------------------------------------------------------------------------------------+
 *| Global Variables                                                                     |
 *+--------------------------------------------------------------------------------------+ */

  float uptime = 0;

  float battLevel_non_filtered 		= 0;		
  float battLevel_filtered 		= 0;			  //	Filtered signal
  float battLevel_filtered_prev	= 0;			// 	Filtered signal from previous interation

/*+--------------------------------------------------------------------------------------+
 *| MQTT constants                                                                       |
 *+--------------------------------------------------------------------------------------+ */

#ifdef USE_MQTT
  #include <PubSubClient.h>
  // update below mqtt broker parameters

  const char *ID = "DuinoFarmDev";                        // Name of our device, must be unique
  const char *TOPIC = "DuinoFarm/data";                   // Topic to subcribe to
  //const char* BROKER_MQTT = "mqtt.eclipseprojects.io";  // MQTT Cloud Broker URL
  const char* BROKER_MQTT = "broker.hivemq.com";

  //#define mqtt_server "broker.hivemq.com"
  //#define mqtt_port 1883
  //#define mqtt_user "your_mqtt_username"
  //#define mqtt_password "your_super_secret_mqtt_password"
  //#define mqtt_temperature_delta_time 900000

  // update humidity_topic to your mqtt humidity topic
  //#define humidity_topic "sensor/humidity"
  // update temperature_topic to your mqtt temperature topic
  //#define temperature_topic "sensor/temperature"
  
  //WiFiClient espClient;
  //PubSubClient mqttClient(espClient);
  
  WiFiClient wclient;
  PubSubClient client(wclient);                         // Setup MQTT client

/*+--------------------------------------------------------------------------------------+
 *| Reconnect to MQTT client                                                             |
 *+--------------------------------------------------------------------------------------+ */
 
  void mqttReconnect() {

    while (!client.connected()) {                       /* Loop until we're reconnected */
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID)) {
      Serial.println("connected");
      Serial.print("Publishing to: ");
      Serial.println(TOPIC);
      Serial.println('\n');

    } else {
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      setup_wifi();
    }
  }
    // Loop until we're reconnected
    /*while (!mqttClient.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (mqttClient.connect("ESP8266Client", mqtt_user, mqtt_password)) {
        Serial.println("connected");
      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }*/

  }

/*+--------------------------------------------------------------------------------------+
 *| Serialize JSON and publish MQTT                                                      |
 *+--------------------------------------------------------------------------------------+ */

void SerializeAndPublish() {

  if (!client.connected())                            /* Reconnect if connection to MQTT is lost */
  {    reconnect();      }

  client.loop();                                      /* MQTT */

  char buff[10];                                      /* Buffer to allocate decimal to string conversion */
  char buffer[256];                                   /* JSON serialization */
  
    StaticJsonDocument<256> doc;                      /* See ArduinoJson Assistant V6 */
    
      doc["Device"] = "DuinoFarmMaster";
      doc["Version"] = "DuinoFarm100";
      doc["RSSI (db)"] = WiFi.RSSI();
      doc["IP"] = WiFi.localIP();
      doc["LastRoll"] = DateAndTime();
      doc["UpTime (h)"] = uptime;

      doc["Hashrate"] = String(hashrate / 1000);
      doc["Difficulty"] = String(difficulty / 1000);
      doc["SahreCount"] = String(share_count);
      doc["BattLevel"] = String(battLevel_filtered);
    
    serializeJson(doc, buffer);
      Serial.println("JSON Payload:");
    serializeJsonPretty(doc, Serial);                 /* Print JSON payload on Serial port */        
      Serial.println("");
                         
      Serial.println("Sending message to MQTT topic");
    client.publish(TOPIC, buffer);                    /* Publish data to MQTT Broker */
      Serial.println("");

}

  
  //bool checkBound(float newValue, float prevValue, float maxDiff) {
  //  return !isnan(newValue) &&
  //         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
  //}
  
  //long lastMsg = 0;
  //float diff = 0.01; // change this to the minimum difference considered for update

#endif

/*
#ifdef USE_DHT
  
  float temp = 0.0;
  float hum = 0.0;
  float temp_weight = 0.3; // 1 for absolute new value, 0-1 for smoothing the new reading with previous value
  float temp_min_value = -20.0;
  float temp_max_value = 70.0;
  float hum_weight = 0.3; // 1 for absolute new value, 0-1 for smoothing the new reading with previous value
  float hum_min_value = 0.1;
  float hum_max_value = 100.0;
    
  // Install "DHT sensor library" if you get an error
  #include <DHT.h>
  // Change D3 to the pin you've connected your sensor to
  #define DHTPIN D3
  // Set DHT11 or DHT22 accordingly
  #define DHTTYPE DHT11
  DHT dht(DHTPIN, DHTTYPE);
#endif
*/

/*+--------------------------------------------------------------------------------------+
 *| namespace Duino-Coin heritage                                                        |
 *+--------------------------------------------------------------------------------------+ */

namespace {
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
const bool WEB_DASHBOARD = true;
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
 *| Webserver                                                                            |
 *+--------------------------------------------------------------------------------------+ */

const char WEBSITE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<!--
    Duino-Coin self-hosted dashboard
    MIT licensed
    Duino-Coin official 2019-2022
    https://github.com/revoxhere/duino-coin
    https://duinocoin.com
-->
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Duino-Coin @@DEVICE@@ dashboard</title>
    <link rel="stylesheet" href="https://server.duinocoin.com/assets/css/mystyles.css">
    <link rel="shortcut icon" href="https://github.com/revoxhere/duino-coin/blob/master/Resources/duco.png?raw=true">
    <link rel="icon" type="image/png" href="https://github.com/revoxhere/duino-coin/blob/master/Resources/duco.png?raw=true">
</head>
<body>
    <section class="section">
        <div class="container">
            <h1 class="title">
                <img class="icon" src="https://github.com/revoxhere/duino-coin/blob/master/Resources/duco.png?raw=true">
                @@DEVICE@@ <small>(@@ID@@)</small>
            </h1>
            <p class="subtitle">
                Self-hosted, lightweight, official dashboard for your <strong>Duino-Coin</strong> miner
            </p>
        </div>
        <br>
        <div class="container">
            <div class="columns">
                <div class="column">
                    <div class="box">
                        <p class="subtitle">
                            Mining statistics
                        </p>
                        <div class="columns is-multiline">
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    <span id="hashratex">@@HASHRATE@@</span>kH/s
                                </div>
                                <div class="heading is-size-5">
                                    Hashrate
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@DIFF@@
                                </div>
                                <div class="heading is-size-5">
                                    Difficulty
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@SHARES@@
                                </div>
                                <div class="heading is-size-5">
                                    Shares
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@NODE@@
                                </div>
                                <div class="heading is-size-5">
                                    Node
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="column">
                    <div class="box">
                        <p class="subtitle">
                            Device information
                        </p>
                        <div class="columns is-multiline">
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@DEVICE@@
                                </div>
                                <div class="heading is-size-5">
                                    Device type
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@ID@@
                                </div>
                                <div class="heading is-size-5">
                                    Device ID
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@MEMORY@@
                                </div>
                                <div class="heading is-size-5">
                                    Free memory
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@VERSION@@
                                </div>
                                <div class="heading is-size-5">
                                    Miner version
                                </div>
                            </div>
)====="
/*
#ifdef USE_DHT
"                            <div class=\"column\" style=\"min-width:15em\">"
"                                <div class=\"title is-size-5 mb-0\">"
"                                    @@TEMP@@ °C"
"                                </div>"
"                                <div class=\"heading is-size-5\">"
"                                    Temperature"
"                                </div>"
"                            </div>"
"                            <div class=\"column\" style=\"min-width:15em\">"
"                                <div class=\"title is-size-5 mb-0\">"
"                                    @@HUM@@ %"
"                                </div>"
"                                <div class=\"heading is-size-5\">"
"                                    Humidity"
"                                </div>"
"                            </div>"
#endif
*/
  R"=====(
                        </div>
                    </div>
                </div>
            </div>
            <br>
            <div class="has-text-centered">
                <div class="title is-size-6 mb-0">
                    Hosted on
                    <a href="http://@@IP_ADDR@@">
                        http://<b>@@IP_ADDR@@</b>
                    </a>
                    &bull;
                    <a href="https://duinocoin.com">
                        duinocoin.com
                    </a>
                    &bull;
                    <a href="https://github.com/revoxhere/duino-coin">
                        github.com/revoxhere/duino-coin
                    </a>
                </div>
            </div>
        </div>
        <script>
            setInterval(function(){
                getData();
            }, 3000);
            
            function getData() {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        document.getElementById("hashratex").innerHTML = this.responseText;
                    }
                };
                xhttp.open("GET", "hashrateread", true);
                xhttp.send();
            }
        </script>
    </section>
</body>
</html>
)=====";

ESP8266WebServer server(80);

void hashupdater(){ //update hashrate every 3 sec in browser without reloading page
  server.send(200, "text/plain", String(hashrate / 1000));
  Serial.println("Update hashrate on page");
};

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
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  if (http.begin(client, URL)) {
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

WiFiClient client;
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
 *| Wifi connect                                                                         |
 *+--------------------------------------------------------------------------------------+ */

void setup_wifi() {
  Serial.print("\nConnecting to ");
  Serial.println(SSID);
    WiFi.mode(WIFI_STA);                            // Setup ESP in client mode
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.begin(SSID, PASSWORD);

  int wait_passes = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++wait_passes >= 20) { ESP.restart(); }     // Restart in case of no wifi connection 
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

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
 *| Blink method                                                                         |
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

/*+--------------------------------------------------------------------------------------+
 *| Restart Method                                                                       |
 *+--------------------------------------------------------------------------------------+ */

void RestartESP(String msg) {
  Serial.println(msg);
  Serial.println("Restarting ESP...");
  blink(BLINK_RESET_DEVICE);
  ESP.reset();
}

/*+--------------------------------------------------------------------------------------+
 *| Watchdog                                                                             |
 *+--------------------------------------------------------------------------------------+ */

// Our new WDT to help prevent freezes
// code concept taken from https://sigmdel.ca/michel/program/esp8266/arduino/watchdogs2_en.html
void ICACHE_RAM_ATTR lwdtcb(void) {
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

  while (client.connected()) {
    if (client.available()) {
      client_buffer = client.readStringUntil(END_TOKEN);
      if (client_buffer.length() == 1 && client_buffer[0] == END_TOKEN)
        client_buffer = "???\n"; // NOTE: Should never happen

      break;
    }
    handleSystemEvents();
  }
}

void ConnectToServer() {
  if (client.connected())
    return;

  Serial.println("\n\nConnecting to the Duino-Coin server...");
  while (!client.connect(host, port));

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

void dashboard() {
  Serial.println("Handling HTTP client");

  String s = WEBSITE;
  s.replace("@@IP_ADDR@@", WiFi.localIP().toString());
  
  s.replace("@@HASHRATE@@", String(hashrate / 1000));
  s.replace("@@DIFF@@", String(difficulty / 100));
  s.replace("@@SHARES@@", String(share_count));
  s.replace("@@NODE@@", String(node_id));

  s.replace("@@DEVICE@@", String(DEVICE));
  s.replace("@@ID@@", String(RIG_IDENTIFIER));
  s.replace("@@MEMORY@@", String(ESP.getFreeHeap()));
  s.replace("@@VERSION@@", String(MINER_VER));
/*
#ifdef USE_DHT
  s.replace("@@TEMP@@", String(temp));
  s.replace("@@HUM@@", String(hum));
#endif
*/
  server.send(200, "text/html", s);
}

} // namespace

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

/*+--------------------------------------------------------------------------------------+
 *| Get Date & Time                                                                      |
 *+--------------------------------------------------------------------------------------+ */
 
String DateAndTime(){

    timeClient.setTimeOffset(-10800);                       // Set offset time in seconds to adjust for your timezone, for example:
                                                            // GMT +1 = 3600
                                                            // GMT +8 = 28800
                                                            // GMT -1 = -3600
                                                            // GMT 0 = 0
    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }

  time_t epochTime = timeClient.getEpochTime();              // The time_t type is just an integer. 
                                                             // It is the number of seconds since the Epoch.
  struct tm * tm = localtime(&epochTime);
  char dts[22];
    strftime(dts, sizeof(dts), "%d%b%Y %H-%M-%S", tm);       // https://www.cplusplus.com/reference/ctime/strftime/
  
  return dts;
 
}

/*+--------------------------------------------------------------------------------------+
 *| Setup                                                                                |
 *+--------------------------------------------------------------------------------------+ */

const int enableNext = 5;           // Declare GPIO5 as "enableNext". This pin shall be connected to EN pin of next ESP8266

void setup() {
  Serial.begin(115200);
  Serial.println("\nDuino-Coin " + String(MINER_VER));
  pinMode(LED_BUILTIN, OUTPUT);

    
  pinMode(enableNext, OUTPUT);      // initialize the pin as an output
  digitalWrite(enableNext, LOW);    // Keeps next ESP8266 disabled

  #ifdef USE_MQTT
    //mqttClient.setServer(mqtt_server, mqtt_port);
    Serial.println("Broker MQTT setting server.. ");
      client.setServer(BROKER_MQTT, 1883);

    Serial.println("Starting timeclient server.. "); 
      timeClient.begin();  
  #endif
  
  /*
  #ifdef USE_DHT
    Serial.println("Initializing DHT sensor");
    dht.begin();
    Serial.println("Test reading: " + String(dht.readHumidity()) + "% humidity");
    Serial.println("Test reading: temperature " + String(dht.readTemperature()) + "*C");
  #endif
  */

  // Autogenerate ID if required
  chipID = String(ESP.getChipId(), HEX);
  
  if(strcmp(RIG_IDENTIFIER, "Auto") == 0 ){
    AutoRigName = "ESP8266-" + chipID;
    AutoRigName.toUpperCase();
    RIG_IDENTIFIER = AutoRigName.c_str();
  }

  setup_wifi();
    Serial.println("Rig name: " + String(RIG_IDENTIFIER));
    Serial.println();

    UpdatePool();

  SetupOTA();

  lwdtFeed();
  lwdTimer.attach_ms(LWD_TIMEOUT, lwdtcb);
  if (USE_HIGHER_DIFF) START_DIFF = "ESP8266H";
  else START_DIFF = "ESP8266";

  if(WEB_DASHBOARD) {
    if (!MDNS.begin(RIG_IDENTIFIER)) {
      Serial.println("mDNS unavailable");
    }
    MDNS.addService("http", "tcp", 80);
    Serial.print("Configured mDNS for dashboard on http://" 
                  + String(RIG_IDENTIFIER)
                  + ".local (or http://"
                  + WiFi.localIP().toString()
                  + ")");
    server.on("/", dashboard);
    if (WEB_HASH_UPDATER) server.on("/hashrateread", hashupdater);
    server.begin();
  }

  blink(BLINK_SETUP_COMPLETE);
  digitalWrite(enableNext, HIGH);    // Enables next ESP8266

  #ifdef USE_MQTT
    Serial.print("Initial MQTT publish .. "); 
      SerializeAndPublish();  
  #endif 
}

/*+--------------------------------------------------------------------------------------+
 *| Main Loop                                                                            |
 *+--------------------------------------------------------------------------------------+ */ 

void loop() {
  
  uptime = millis()/3600000;                          // Update uptime 

  br_sha1_context sha1_ctx, sha1_ctx_base;
  uint8_t hashArray[20];
  String duco_numeric_result_str;
  
  // 1 minute watchdog
  lwdtFeed();

  // OTA handlers
  VerifyWifi();
  ArduinoOTA.handle();
  if(WEB_DASHBOARD) server.handleClient();

  ConnectToServer();
  Serial.println("Asking for a new job for user: " + String(USERNAME));

  
  #ifndef USE_DHT
    client.print("JOB," + 
                 String(USERNAME) + SEP_TOKEN +
                 String(START_DIFF) + SEP_TOKEN +
                 String(MINER_KEY) + END_TOKEN);
  #endif
  
  /*
  #ifdef USE_DHT
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    if ((temp >= temp_min_value) && (temp <= temp_max_value)) {
      if ((newTemp >= temp_min_value) && (newTemp <= temp_max_value)) {
        newTemp = temp_weight * newTemp + (1.0f - temp_weight) * temp; // keep weighted measurement value
      } else {
        newTemp = temp; // keep current temp
      }
    } // else - keep newTemp as is

    if ((hum >= hum_min_value) && (hum <= hum_max_value)) {
      if ((newHum >= hum_min_value) && (newHum <= hum_max_value)) {
        newHum = hum_weight * newHum + (1.0 - hum_weight) * hum; // keep weighted measurement value
      } else {
        newHum = hum; // keep current hum
      }
    } // else - keep newHum as is
  #endif
  */
  
  #ifdef USE_MQTT
  
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();
    /*
    #ifdef USE_DHT
    if (checkBound(newTemp, temp, diff)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
    }
    if (checkBound(newHum, hum, diff)) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
    }
    long now = millis();
    if (now - lastMsg > mqtt_temperature_delta_time) {
      lastMsg = now;
      mqttClient.publish(temperature_topic, String(temp).c_str(), true);
      mqttClient.publish(humidity_topic, String(hum).c_str(), true); 
    }
    #endif
    */

  #endif
  
  /*
  #ifdef USE_DHT

    Serial.println("DHT readings: " + String(temp) + "*C, " + String(hum) + "%");
    client.print("JOB," + 
                 String(USERNAME) + SEP_TOKEN +
                 String(START_DIFF) + SEP_TOKEN +
                 String(MINER_KEY) + SEP_TOKEN +
                 String(temp) + "@" + String(hum) + END_TOKEN);
  #endif
  */

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
      client.print(String(duco_numeric_result)
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

/*+--------------------------------------------------------------------------------------+
 *| Battery management                                                                   |
 *+--------------------------------------------------------------------------------------+ */

  float alpha 				= 0.2;			                                        //	Filter coefficient.  1 means no filter. As lower the value, more filtered is the signal
  battLevel_non_filtered = analogRead(A0);	                              // Read the battery voltage
  
  battLevel_filtered = alpha * battLevel_non_filtered + (1-alpha) * battLevel_filtered_prev;  // First-order filter
      
  battLevel_filtered_prev = battLevel_filtered;

  Serial.println("battLevel_non_filtered: " + String(battLevel_non_filtered));
  Serial.println("battLevel_filtered:     " + String(battLevel_filtered));
  
  #ifdef USE_MQTT
    SerializeAndPublish();
  #endif
}