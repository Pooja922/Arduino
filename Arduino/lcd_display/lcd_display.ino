#include <gfxfont.h>

#include <Adafruit_GFX.h>
#include <Fonts/FreeSans18pt7b.h>
#include <MCUFRIEND_kbv.h>

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define DARKGREY 0x7BEF /* 128, 128, 128 */
#define NAVY 0x000F /* 0, 0, 128 */
#define DARKGREEN 0x03E0 /* 0, 128, 0 */
#define DARKCYAN 0x03EF /* 0, 128, 128 */
#define MAROON 0x7800 /* 128, 0, 0 */
#define PURPLE 0x780F /* 128, 0, 128 */
#define OLIVE 0x7BE0 /* 128, 128, 0 */
#define LIGHTGREY 0xC618 /* 192, 192, 192 */
#define DARKGREY 0x7BEF /* 128, 128, 128 */
#define ORANGE 0xFD20 /* 255, 165, 0 */
#define GREENYELLOW 0xAFE5 /* 173, 255, 47 */
#define PINK 0xF81F

MCUFRIEND_kbv tft;

void setup() {

  tft.reset();
  tft.begin(0x9486);
  
drawHomeScreen();
}

void loop(){
  

}

void drawHomeScreen() {
  
  tft.setRotation(0);     //vertical
  tft.fillScreen(BLACK);
  
  //med name and dosage title
  tft.setCursor(5, 280);
  tft.setTextColor(CYAN);
  tft.setTextSize(1);
  tft.setFont(&FreeSans18pt7b);
  tft.print("Name"); 

  //med name and dosage value
  tft.setCursor(5, 330);
  tft.setTextColor(DARKCYAN);
  tft.setTextSize(1);
  tft.setFont(&FreeSans18pt7b);
  tft.print("Aspirin"); 

  //med instruction title
  tft.setCursor(5, 390);
  tft.setTextColor(CYAN);
  tft.setTextSize(1);
  tft.setFont(&FreeSans18pt7b);
  tft.print("Instruction   Dosage");

  //med instruction value
  tft.setCursor(5, 430);
  tft.setTextColor(DARKCYAN);
  tft.setTextSize(1);
  tft.setFont(&FreeSans18pt7b);
  tft.print("After Eating    1"); 
}
