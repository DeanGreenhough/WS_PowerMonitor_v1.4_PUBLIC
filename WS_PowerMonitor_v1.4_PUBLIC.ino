
/* v1.0 ESP32, , MQTT, INA219  POWER MONITOR FOR WS PROJECT */
// v1.1 ADDED POWER_MONITOR_PIN
// v1.2 ADDED LOOPTIMER AND REDUCE ANY DELAYS
// v1.3 ADDED LOOP COUNTER FOR if LOOP TO SEE HOW MANY MEAUSREMENTS ARE TAKEN IN A GIVEN CYCLE
// v1.4 COMMENTED OUT ANY PRINT STATEMENTS - SAMPLES GONE FROM MAX 33 TO 66
// v1.4 CLEANED UP CODE & MQTT delay SET TO 3mS AS MOST STABLE READINGS
// v1.4_PUBLIC  ***** REMOVE PASSWORDS FOR PUBLIC DOMAIN *****

#include <WiFi.h>
#include <PubSubClient.h>
//INA219
#include <Wire.h>
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;

//MQTT DELAY
#define mqtt_delay_samples          5                    // CHANGE SPEED OF DELAY 3 Secs WORKS
#define DEBUG                       1                    // DEBUG OUTPUT 
//bool debug = true;							                       // MQTT SERIEL DEBUG
#define wifi_ssid                   "L*******s"          // wifi ssid
#define wifi_password               "F*******5"          // wifi password
#define mqtt_server                 "192.168.0.***"      // server name or IP
#define mqtt_user                   "*****"              // username
#define mqtt_password               "*****"              // password
// #DEFINE MQTT TOPICS
#define busvoltage_topic            "powerMonitor/busvoltage"
#define current_mA_topic            "powerMonitor/current_mA"
#define power_mW_topic              "powerMonitor/power_mW"
#define samples_topic               "powerMonitor/samples"
#define looptime_topic              "powerMonitor/looptime"
//i2c
uint32_t currentFrequency;

int POWER_MONITOR_PIN      = 0;      //DETECT WHEN DUT POWERING UP
//float shuntvoltage       = 0;
float busvoltage           = 0;
float current_mA           = 0;
float power_mW             = 0;
//float loadvoltage        = 0;
static int numLoops        = 0;

//LoopTimer
unsigned long startMillis;         //LOOP TIMER
unsigned long currentMillis;       //LOOP TIMER

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
              Serial.begin(115200);
              if (DEBUG)Serial.println(__FILE__);
              if (DEBUG)Serial.println(__DATE__);
              if (DEBUG)Serial.println(__TIME__);
  
              //uint32_t currentFrequency;
              setup_wifi();                        
              ina219.begin();
              ina219.setCalibration_16V_400mA();
            }


void loop() {
              long loopTime = 0;
              startMillis = millis();      //START LOOPTIME          
              POWER_MONITOR_PIN = analogRead (A0);
            
         if (POWER_MONITOR_PIN > 4000) 
            {
              busvoltage   = ina219.getBusVoltage_V();
              current_mA   = ina219.getCurrent_mA();
              power_mW     = ina219.getPower_mW();  
              //CHECK SERVER CONNECTION
              client.setServer(mqtt_server, 1883);
              if (!client.connected()) {  reconnect();  }          
              // PUBLISH to MQTT
              client.publish(busvoltage_topic, String(busvoltage).c_str(), true);
              delay(mqtt_delay_samples);
              client.publish(current_mA_topic, String(current_mA).c_str(), true);
              delay(mqtt_delay_samples);
              client.publish(power_mW_topic, String(power_mW).c_str(), true);
              delay(mqtt_delay_samples);
              numLoops++;  //LOOP COUNTER
              client.publish(samples_topic, String(numLoops).c_str(), true);
              delay(mqtt_delay_samples);
              currentMillis = millis();   //END LOOPTIME
              loopTime = (currentMillis - startMillis) ;
              client.publish(looptime_topic, String(loopTime).c_str(), true);
              delay(10);
            }

      else  {
              //numLoops        = 0;
              //currentMillis = millis();  //END LOOPTIME
              //loopTime = (currentMillis - startMillis) ;
              //client.publish(looptime_topic, String(loopTime).c_str(), true);
              //delay(mqtt_delay_samples);
            }

}


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(10);
    Serial.print(".");
  }

 Serial.println("");
 Serial.println("WiFi is OK ");
 Serial.print("IP address is: ");
 Serial.print(WiFi.localIP());
 Serial.println("");
}

//Reconnect to wifi if connection is lost
void reconnect() {

  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker ...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("OK");
    } else {
      Serial.print("[Error] Not connected: ");
      Serial.print(client.state());
      Serial.println("Wait 5 seconds before retry.");
      delay(500);
    }
  }
}
