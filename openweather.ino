/* Weather (Okoppe, Hokkaido, Japan) */
const char OpenWeatherURL[] = "api.openweathermap.org";
const char OpenWeatherRequest[] = "/data/2.5/forecast?lat=___LATITUDE___&lon=___LONGITUDE___&appid=___API KEY___&cnt=6&units=metric";

///////////////////
/**** Weather ****/
///////////////////
String myWeatherFunction()
{
  Serial.print("Downloading the latest weather forecast...");
  WiFiClient client;
  if (!client.connect(OpenWeatherURL, 80))
  {
    Serial.print("!!!\nCould not connect to ");
    Serial.println(OpenWeatherURL);

    // Cannot Connect
    return "";
  }

  // Get the weather
  client.print("GET ");
  client.print(OpenWeatherRequest);
  client.print(" HTTP/1.1\r\nHost: ");
  client.print(OpenWeatherURL);
  client.print("\r\nConnection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println("!!!\nClient timed out!");

      client.stop();
      return "";
    }
    delay(100);
    Serial.print(".");
  }

  // Read all the lines of the reply from server and print them to Serial
  String response = "";
  bool foundContent = false;
  while (client.available())
  {
    String thisline = client.readStringUntil('\n');
    if (foundContent)
    {
      response += thisline;
    }
    else if (thisline.compareTo("\r") == 0)
    {
      foundContent = true;
    }
  }
  Serial.println("\nWeather forecast retrieved!");
  Serial.print(response);

  return response;
}