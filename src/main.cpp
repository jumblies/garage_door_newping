/* Todo: fix OTA confirmation
Ready to upload to git.
updated 2018 06 05 with newPing library that dramatically decreased false
readings.  With the default of 5 only had 1 false in overnight period.
last mod will increase to 10 pings, set interval to 15 sec ~line 26 to see if
it is possible to completely filter erroneousreadings
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "secrets.h"
#include <PubSubClient.h>
#include <NewPing.h>
// NewPing Library Defines
#define TRIGGER_PIN 4    // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 5       // Arduino pin tied to echo pin on the ultrasonic sensor.
#define TRIGGER_PIN2 14  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN2 16     // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
// NewPing Instantiation
NewPing sonar1(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
NewPing sonar2(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE);
// NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
WiFiClient espClient;
PubSubClient client(espClient);
//  _______   _
// |__   __| (_)
//   | |     _   _ __ ___     ___   _ __
//   | |    | | | '_ ` _ \   / _ \ | '__|
//   | |    | | | | | | | | |  __/ | |
//   |_|    |_| |_| |_| |_|  \___| |_|
uint16_t interval = 1000 * 15;

float reading1, reading2;

/*NewPing Functions to read G and K garage door sensors
// will have to check to see if they interfere with each other and need
// to temporally separated.*/
void readG()
{
  reading1 = sonar1.convert_cm(sonar1.ping_median(10));
  client.publish(topic1, String(reading1).c_str(), false);
}
void readK()
{
  reading2 = sonar2.convert_cm(sonar2.ping_median(10));
  client.publish(topic2, String(reading2).c_str(), false);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("GDoorSensor", NULL, NULL, LWTTopic, 0, 0, LWTMessage))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  //MQTT Setup
  client.setServer(mqtt_server, 1883);

  //OTA setup
  ArduinoOTA.setHostname("GARDOOR");
  // Delay necessary to get ack from upload
  ArduinoOTA.onStart([]() {});
  ArduinoOTA.onEnd([]() { delay(500); });

  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    ESP.restart();
  });
  /* setup the OTA server */
  ArduinoOTA.begin();
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();

  static uint32_t tick1 = 0;
  if (millis() - tick1 < interval)
  {
    return;
  }
  readG();
  readK();
  tick1 = millis(); // reset the timer

} //End of Loop
