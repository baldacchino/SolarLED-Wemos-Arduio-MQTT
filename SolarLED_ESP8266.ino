#include <PubSubClient.h>
#include <ESP8266WiFi.h>

int feed_in_red_light_pin= 5;
int feed_in_green_light_pin = 4;
int feed_in_blue_light_pin = 0;

int consumption_red_light_pin= 2;
int consumption_green_light_pin = 12;
int consumption_blue_light_pin = 15;



const char* ssid = "Your SSID";
const char* pswd = "Your Password";
const char* mqtt_server = "Your MQTT FQDN or IP";
//const char* topic = "wemos";    // rhis is the [root topic]

//long timeBetweenMessages = 1000 * 20 * 1;

String MQTTTopic;
String MQTTPayload;
String IPAddress;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
int value = 0;

int status = WL_IDLE_STATUS;     // the starting Wifi radio's status

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pswd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  MQTTTopic = String(topic);
  MQTTPayload = ""; 
  Serial.println("T [" + MQTTTopic +"]");
  for (int i = 0; i < length; i++) {
    MQTTPayload = String(MQTTPayload + (char)payload[i]);
  }
  Serial.println("P [" + MQTTPayload + "]");

}


String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

String composeClientID() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String clientId;
  clientId += "esp-";
  clientId += macToStr(mac);
  return clientId;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = composeClientID() ;
    clientId += "-";
    clientId += String(micros() & 0xff, 16); // to randomise. sort of

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("stat/WemosD1-Study/IpAddress","10.0.0.40");
      //Subscribe
      client.subscribe("homeassistant/sensor/metering_active_power_feed_l1/state");
      Serial.println("");
      Serial.print("Subscribed to : homeassistant/sensor/metering_active_power_feed_l1/state ");
      client.subscribe("homeassistant/sensor/energy_house_consumption/state");
      Serial.println("");
      Serial.print("Subscribed to : homeassistant/sensor/energy_house_consumption/state ");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.print(" wifi=");
      Serial.print(WiFi.status());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(feed_in_red_light_pin, OUTPUT);
  pinMode(feed_in_green_light_pin, OUTPUT);
  pinMode(feed_in_blue_light_pin, OUTPUT);
}

void loop() {
  // confirm still connected to mqtt server
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (MQTTTopic == "homeassistant/sensor/metering_active_power_feed_l1/state") {
      
    if (MQTTPayload.toInt() == 0)  {
      feed_in_RGB_color(255, 0, 0); // Red
    // Serial.println("Changing to red");
    }
      else if (MQTTPayload.toInt() > 0 && MQTTPayload.toInt() < 251) {
      feed_in_RGB_color(255, 0, 255); // Magenta = Error
      //  Serial.println("Changing to magenta");
  
    }
    else if (MQTTPayload.toInt() > 250 && MQTTPayload.toInt() < 501) {
      feed_in_RGB_color(0, 0, 255); // Blue
      //  Serial.println("Changing to blue");
  
    }
    else if (MQTTPayload.toInt() > 500 && MQTTPayload.toInt() < 1001) {
      feed_in_RGB_color(0, 255, 255); // Cyan 
      //  Serial.println("Changing to cyan");
    }
    else if (MQTTPayload.toInt() > 1000 && MQTTPayload.toInt() <= 2500) {
        
        feed_in_RGB_color(255, 255, 0); // Yellow
    // Serial.println("Changing to yellow");
    }
    else if (MQTTPayload.toInt() > 2500) {
        feed_in_RGB_color(0, 255, 0); // Green
      //  Serial.println("Changing to green");
      }
  }

// House Consumtpion
   if (MQTTTopic == "homeassistant/sensor/energy_house_consumption/state") {
      
    if (MQTTPayload.toInt() > 0 && MQTTPayload.toInt() < 401) {
      consumption_RGB_color(0, 255, 0); // Green
 
  
    }
    else if (MQTTPayload.toInt() > 400 && MQTTPayload.toInt() < 501) {
      // Serial.println("Changing to yellow");
        consumption_RGB_color(255, 255, 0); // Yellow
  
    }
      else if (MQTTPayload.toInt() > 500 && MQTTPayload.toInt() < 1001) {
              consumption_RGB_color(0, 255, 255); // Cyan
        //  Serial.println("Changing to cyan");

     
    }
    else if (MQTTPayload.toInt() > 1000 && MQTTPayload.toInt() <= 2001) {
            consumption_RGB_color(0, 0, 255); // Blue
      //  Serial.println("Changing to blue");


    }
    else if (MQTTPayload.toInt() > 2000 && MQTTPayload.toInt() <= 3001) {
               consumption_RGB_color(255, 0, 255); // Magenta
    }
    else if (MQTTPayload.toInt() > 3000) {
      // Serial.println("Changing to red");
        
        consumption_RGB_color(255, 0, 0); // Red
      //  Serial.println("Changing to red");
      }
  }

}



void feed_in_RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(feed_in_red_light_pin, red_light_value);
  analogWrite(feed_in_green_light_pin, green_light_value);
  analogWrite(feed_in_blue_light_pin, blue_light_value);
 }


 void consumption_RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(consumption_red_light_pin, red_light_value);
  analogWrite(consumption_green_light_pin, green_light_value);
  analogWrite(consumption_blue_light_pin, blue_light_value);
 }
