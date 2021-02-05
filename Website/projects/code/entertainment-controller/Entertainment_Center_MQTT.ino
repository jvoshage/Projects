#include <IRLib.h>
#include <IRLibMatch.h>


#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ESP8266WiFi.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define WIFI_SSID "**********"
#define WIFI_PASS "**********"
#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "**********"
#define MQTT_PASS "**********"
#define IR_LED 2

WiFiClient client;
IRsend irsend(IR_LED);
MyCustomSend DTV_Sender;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);
Adafruit_MQTT_Subscribe controller = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/entertainment-center");

int khz = 38; //Carrier frequency for controling the samsung TV

void setup() {
  Serial.begin(115200);
  irsend.begin();

  //Connect to WiFi
  Serial.print("\n\nConnecting WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("Connected");

  //Subscribe to the entertainment center topic
  mqtt.subscribe(&controller);
  pinMode(2, OUTPUT);
}

void loop()
{
  //Connect/Reconnect to MQTT
  MQTT_connect();

  //Read from our subscription queue until we run out, or
  //wait up to 5 seconds for subscription to update
  Adafruit_MQTT_Subscribe * subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    //If we're in here, a subscription updated...
    if (subscription == &controller)
    {
      //Print the new value to the serial monitor
      Serial.print("MQTT sent: ");
      Serial.println((char*) controller.lastread);
      //Convert the incoming string into an int
      //Process the resulting number into commands
      process(atoi((char*) controller.lastread));
    }

    // ping the server to keep the mqtt connection alive
    if (!mqtt.ping())
    {
      mqtt.disconnect();
    }
  }
}

//===========================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===========================================================================================================

void MQTT_connect()
{
  int8_t ret;
  // Stop if already connected
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}

//===========================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===========================================================================================================
void runit(int device, int command)
{

  switch (device)
  {
    case 1: {
        //TV
        switch (command)
        {
          case 0:
            {
              //Power
              uint16_t irSignal[] = {4450, 4350, 600, 1650, 600 , 1600, 600 , 1600, 600, 500, 600, 550, 600, 500, 600, 500, 600, 500, 600, 1600, 600, 1650, 600, 1600, 600, 500, 600, 500, 600, 500, 600, 550, 550, 550, 600, 500, 600, 1600, 600, 500, 600, 500, 600, 550, 600, 500, 600, 500, 600, 500, 600, 1600, 600, 500, 600, 1650, 600, 1600, 600, 1600, 600, 1650, 600, 1600, 600, 1600, 600}; //AnalysIR Batch Export (IRremote) - RAW
              irsend.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), khz); //Note the approach used to automatically calculate the size of the array.
              break;
            }
          case 1:
            {
              //Volume Up
              uint16_t irSignal[] = {4500, 4350, 650, 1550, 650, 1600, 600, 1600, 650, 450, 650, 450, 650, 450, 650, 500, 600, 500, 650, 1550, 650, 1550, 650, 1600, 600, 500, 650, 450, 650, 450, 650, 450, 650, 450, 650, 1600, 600, 1600, 650, 1550, 650, 450, 650, 500, 600, 500, 650, 450, 650, 450, 650, 450, 650, 450, 650, 500, 600, 1600, 650, 1550, 650, 1600, 650, 1550, 600, 1600, 650}; //AnalysIR Batch Export (IRremote) - RAW
              irsend.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), khz); //Note the approach used to automatically calculate the size of the array.
              break;
            }
          case 2:
            {
              //Volume Down
              uint16_t irSignal[] = {4450, 4350, 650, 1600, 600, 1600, 650, 1550, 650, 500, 600, 500, 650, 450, 600, 500, 600, 500, 650, 1550, 650, 1600, 600, 1600, 650, 450, 650, 450, 650, 500, 650, 450, 600, 500, 650, 1550, 650, 1550, 650, 500, 600, 1600, 600, 500, 650, 450, 650, 450, 650, 450, 650, 500, 600, 500, 600, 1600, 650, 450, 650, 1600, 600, 1600, 600, 1600, 650, 1600, 600}; //AnalysIR Batch Export (IRremote) - RAW
              irsend.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), khz); //Note the approach used to automatically calculate the size of the array.
              break;
            }
          case 3:
            {
              //Mute
              uint16_t irSignal[] = {4450, 4400, 600, 1600, 650, 1550, 650, 1600, 600, 500, 600, 500, 600, 500, 650, 450, 650, 450, 650, 1600, 600, 1600, 650, 1550, 650, 500, 600, 500, 600, 500, 600, 500, 600, 500, 600, 1600, 650, 1600, 600, 1600, 600, 1600, 650, 500, 600, 500, 600, 500, 600, 500, 600, 500, 650, 450, 650, 450, 650, 500, 600, 1600, 600, 1600, 650, 1600, 600, 1600, 600}; //AnalysIR Batch Export (IRremote) - RAW
              irsend.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), khz); //Note the approach used to automatically calculate the size of the array.
              break;
            }
        }
        break;
      }
    case 2: {
        //Receiver
        switch (command)
        {
          case 0:
            //Power
            irsend.sendNEC(0x7e8154ab, 32);
            break;
          case 6:
            //Volume Up
            irsend.sendNEC(0x5EA158A7, 32);
            break;
          case 7:
            //Volume Down
            irsend.sendNEC(0x5EA1D827, 32);
            break;
          case 1:
            //HDMI 1
            irsend.sendNEC(0x5EA1E21C, 32);
            break;
          case 2:
            //HDMI 2
            irsend.sendNEC(0x5EA152AC, 32);
            break;
          case 3:
            //HDMI 3
            irsend.sendNEC(0x5EA1B24C, 32);
            break;
          case 4:
            //HDMI 4
            irsend.sendNEC(0x5EA10AF4, 32);
            break;
          case 5:
            //HDMI 5
            irsend.sendNEC(0x5EA10EF0, 32);
            break;
        }
        break;
      }
    case 3: {
        //Blueray
        switch (command)
        {
          case 0:
            //Power
            irsend.sendSony(0xA8B47, 20);
            break;
          case 1:
            //Play/Pause
            irsend.sendSony(0x58B47, 20);
            break;
          case 2:
            //Stop
            irsend.sendSony(0x18B47, 20);
            break;
        }
        break;
      }
    case 4:
      { //Directv
        switch (command)
        {
          case 0:
            {
              //Power
              DTV_Sender.send(DIRECTV,0xC105,16)
            }
            break;
          case 1:
            {
              //Guide
              uint16_t irSignal[] = {5950, 1150, 1250, 1100, 650, 500, 700, 500, 1250, 500, 1300, 500, 650, 500, 700, 500, 650, 550, 650};
              irsend.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), khz);
            }
            break;
          case 2:
            {
              //Play/Pause
              uint16_t irSignal[] = {5900, 1200, 1200, 1150, 650, 500, 650, 550, 1200, 1150, 650, 550, 1250, 500, 650, 550, 600, 1150, 650};
              irsend.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), khz);
            }
            break;
          case 3:
            {
              //Info
              uint16_t irSignal[] = {5950, 1150, 1200, 1150, 650, 550, 600, 550, 1250, 550, 1250, 1100, 1200, 550, 650, 1150, 650, 1100, 650};
              irsend.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), khz);
            }
            break;
          case 4:
            {
              //Exit
              uint16_t irSignal[] = {5900, 1150, 1250, 1150, 650, 500, 650, 550, 1200, 550, 650, 1150, 1200, 550, 1200, 1150, 1250, 1100, 650};
              irsend.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), khz);
            }
            break;
        }
      }
      break;
    case 5:
      { //Roku
        switch (command)
        {
          case 0:
            //Roku Back
            irsend.sendNEC(0x57436699, 32);
            break;
          case 1:
            //Roku Home
            irsend.sendNEC(0x5743C03F, 32);
            break;
          case 2:
            //Play/Pause
            irsend.sendNEC(0x574332CD, 32);
            break;
          case 3:
            //Netflix
            irsend.sendNEC(0x5743D22D, 32);
            break;
          case 4:
            //Hulu
            irsend.sendNEC(0x5743B24D, 32);
            break;
        }
      }
  }
}

//===========================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===========================================================================================================

void process(long input)
{
  Serial.print("\nInput: ");
  Serial.println(input);
  int chunk1 = input % 100;
  runit(chunk1 / 10, chunk1 % 10);
  Serial.print("Device: ");
  Serial.print(chunk1 / 10);
  Serial.print("  Button: ");
  Serial.println(chunk1 % 10);
  input /= 100;
  if (input > 0)
  {
    process(input);
  }
}



