bool LFWLL_Increasing = true;
void lookingForWifiLightingLoop()
{
  // Set new brightness
  int16_t newBrightness;
  if (LFWLL_Increasing)
  {
    newBrightness = Lighting.getBrightness() + 10;
    if (newBrightness > 255)
    {
      newBrightness = 255;
      LFWLL_Increasing = false;
    }
  }
  else
  {
    newBrightness = Lighting.getBrightness() - 10;
    if (newBrightness < 0)
    {
      newBrightness = 0;
      LFWLL_Increasing = true;
    }
  }
  Lighting.setBrightness(newBrightness);

  // Turn all LEDs blue and show
  for (int i = 0; i < NumLeds; i++)
  {
    Lighting.setPixelColor(i, Lighting.Color(0, 0, 255));
  }
}