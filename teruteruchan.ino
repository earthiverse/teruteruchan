#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <time.h>

// Notes:
// Some code from https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/WiFiClient/WiFiClient.ino
// Some code from https://bblanchon.github.io/ArduinoJson/assistant/

/*** Variables ***/
/* Wifi */
const int WifiLedPin = 2;
const char Hostname[] = "TeruTeruChan";
const char WifiNetwork[] = "";
const char WifiPassword[] = "";
bool Connected = false;
/* LED */
const int LightingLedPin = 5;
const int NumStrips = 4;
const int LedsPerStrip = 8;
const int NumLeds = NumStrips * LedsPerStrip;
Adafruit_NeoPixel Lighting = Adafruit_NeoPixel(NumLeds, LightingLedPin, NEO_GRB + NEO_KHZ800);

//////////////////////
/**** Essentials ****/
//////////////////////
void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(WifiLedPin, OUTPUT);
  digitalWrite(WifiLedPin, LOW);

  Lighting.begin();
  Lighting.show();

  randomSeed(analogRead(0));

  if (connectToWifi())
  {
    //updateTime();
  }
  else
  {
    // Didn't connect
  }
  updateTime();
}

void loop()
{
  while (true)
  {
    // Get the weather
    const size_t bufferSize = 16 * JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(16) + 8 * JSON_OBJECT_SIZE(0) + 40 * JSON_OBJECT_SIZE(1) + 17 * JSON_OBJECT_SIZE(2) + 16 * JSON_OBJECT_SIZE(4) + 2 * JSON_OBJECT_SIZE(5) + 32 * JSON_OBJECT_SIZE(8) + 5140;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    String weatherJSON = myWeatherFunction();
    JsonObject &root = jsonBuffer.parseObject(weatherJSON.c_str());
    JsonArray &list = root["list"];
    JsonObject &list0 = list[0];
    JsonObject &list0_weather0 = list0["weather"][0];
    int list0_weather0_id = list0_weather0["id"];
    const char *list0_weather0_main = list0_weather0["main"];
    const char *list0_weather0_description = list0_weather0["description"];
    const char *list0_dt_txt = list0["dt_txt"];

    Serial.print("\nThe weather for ");
    Serial.print(list0_dt_txt);
    Serial.print(" is: ");
    Serial.println(list0_weather0_description);

    unsigned long timeout = millis();
    WiFi.forceSleepBegin();
    while (true)
    {
      switch (list0_weather0_id)
      {
      case 500: // Light rain
        LightRainAnimation();
        break;
      default:
        // Other
        break;
      }
      // Break out after 30 minutes to get updated weather
      if (millis() - timeout > 1800000)
      {
        WiFi.forceSleepWake();
        break;
      }
    }
  }
}

////////////////////
/**** Wireless ****/
////////////////////
bool connectToWifi()
{
  Serial.print("Connecting to ");
  Serial.print(WifiNetwork);
  Serial.print("...");
  WiFi.mode(WIFI_STA);
  WiFi.hostname(Hostname);
  WiFi.begin(WifiNetwork, WifiPassword);
  unsigned long timeout = millis();
  while (true)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Connected = true;
      break;
    }
    // TODO: LED Fun
    //digitalWrite(WifiLedPin, ledStatus);
    //ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    lookingForWifiLightingLoop();
    Serial.print(".");
    if (millis() - timeout > 5000)
    {
      Serial.println("!!!\nTimeout! Could not update time!");
      break;
    }
  }

  // TODO: Animation if not connected
  if (Connected)
  {
    Serial.print("\nConnected!\nIP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("!!!\nCould not connect to WiFi network!");
    noWifiLightingAnimation();
  }
  return Connected;
}

////////////////
/**** Time ****/
////////////////
void updateTime()
{
  Serial.print("Updating time...");
  unsigned long timeout = millis();
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (!time(nullptr))
  {
    Serial.print(".");
    delay(250);
    if (millis() - timeout > 5000)
    {
      Serial.println("!!!\nTimeout! Could not update time!");
      break;
    }
  }
  Serial.print("The time is (UTC): ");
  time_t now = time(nullptr);
  Serial.print(ctime(&now));
}

////////////////////
/**** Lighting ****/
////////////////////
int LFWLLIteration = 0;
void lookingForWifiLightingLoop()
{
  Lighting.setBrightness(50);

  if (LFWLLIteration < 8)
  {
    // Turn on the next LED
    for (int i = 0; i < NumStrips; i++)
    {
      Lighting.setPixelColor(i * LedsPerStrip + LFWLLIteration, Lighting.Color(0, 0, 150));
    }
  }
  else
  {
    // Turn off the next LED
    for (int i = 0; i < NumStrips; i++)
    {
      Lighting.setPixelColor(i * LedsPerStrip + LFWLLIteration - 8, Lighting.Color(0, 0, 0));
    }
  }
  Lighting.show();
  delay(50);

  // Increment or Loop
  if (LFWLLIteration == 15)
  {
    LFWLLIteration = 0;
  }
  else
  {
    LFWLLIteration++;
  }
}

void noWifiLightingAnimation()
{
  Lighting.setBrightness(50);

  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 255; j += 10)
    {
      Lighting.setBrightness(j);
      for (int i = 0; i < NumLeds; i++)
      {
        Lighting.setPixelColor(i, Lighting.Color(255, 0, 0));
      }
      Lighting.show();
      delay(10);
    }
    for (int j = 255; j > 0; j -= 10)
    {
      Lighting.setBrightness(j);
      for (int i = 0; i < NumLeds; i++)
      {
        Lighting.setPixelColor(i, Lighting.Color(255, 0, 0));
      }
      Lighting.show();
      delay(10);
    }
  }
  Lighting.setBrightness(255);
  for (int i = 0; i < NumLeds; i++)
  {
    Lighting.setPixelColor(i, Lighting.Color(0, 0, 0));
  }
  Lighting.show();
}

void LightRainAnimation()
{
  // Turn off lights
  Lighting.show();
  Lighting.setBrightness(255);
  Lighting.show();

  // Choose 4 random LEDs, fade it down, then repeat 100 times
  for (int i = 0; i < 100; i++)
  {
    int randomLED_1 = random(0, NumLeds);
    int randomLED_2 = random(0, NumLeds);
    int randomLED_3 = random(0, NumLeds);
    int randomLED_4 = random(0, NumLeds);
    int randomLED_5 = random(0, NumLeds);
    int randomLED_6 = random(0, NumLeds);
    int randomLED_7 = random(0, NumLeds);
    int randomLED_8 = random(0, NumLeds);
    int brightness = 0;
    // Fade in (fast)
    while (brightness < 255)
    {
      brightness += random(10, 15);
      if (brightness > 255)
      {
        brightness = 255;
      }
      Lighting.setPixelColor(randomLED_1, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_2, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_3, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_4, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_5, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_6, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_7, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_8, Lighting.Color(0, 0, brightness));
      Lighting.show();
      delay(25);
    }

    // Fade out (slow)
    while (brightness > 0)
    {
      brightness -= random(0, 5);
      if (brightness < 0)
      {
        brightness = 0;
      }
      Lighting.setPixelColor(randomLED_1, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_2, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_3, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_4, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_5, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_6, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_7, Lighting.Color(0, 0, brightness));
      Lighting.setPixelColor(randomLED_8, Lighting.Color(0, 0, brightness));
      Lighting.show();
      delay(25);
    }
  }
}
