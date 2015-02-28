#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(pinDisplayReset);

void initialiseDisplay()
{
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
   display.clearDisplay();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0,0);
   display.print("RPM: ");
   display.setCursor(0,16);
   display.print("CPU: ");
   
   switch(configPage1.displayType)
   {
     case 1:
       display.SSD1306_SETCOMPINS_V = 0x02;
      break;
    case 2:
       display.SSD1306_SETCOMPINS_V = 0x12;
      break;
     case 3:
       display.SSD1306_SETCOMPINS_V = 0x12;
      break;
     case 4:
       display.SSD1306_SETCOMPINS_V = 0x12;
      break; 
   }
}

void updateDisplay()
{
  display.clearDisplay();
   //display.setCursor(0,0);
   //display.print("RPM: ");
   display.setCursor(64,0);
   display.print("PW: ");
   display.setCursor(0,11);
   display.print("CPU: ");
  /*
  display.setCursor(28,0);
  display.print(currentStatus.RPM); 
  display.setCursor(92,0);
  display.print((currentStatus.PW));*/
  display.setCursor(28,11);
  display.print(currentStatus.loopsPerSecond); 
  
  int barWidth = ldiv(((unsigned long)currentStatus.RPM * 128), 9000).quot;
  //int barWidth = map(currentStatus.RPM, 0, 9000, 0, 128);
  display.fillRect(0, 20, barWidth, 10, 1);
  
  display.display();
}
