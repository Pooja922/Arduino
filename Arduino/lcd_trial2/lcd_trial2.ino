#include <gfxfont.h>

#include <Adafruit_GFX.h>
#include <Fonts/FreeSans18pt7b.h>
#include <MCUFRIEND_kbv.h>


MCUFRIEND_kbv tft;

int hr,mnt;

#define BLACK   0x0000
#define BLUE    0x001F

void setup(){
  tft.reset();
  tft.begin(0x9486);
  hr=6;
  mnt=2;
  drawTime();
}

void loop(){
  delay(60000);
  mnt=mnt+1;
  if(mnt==60){
    hr=hr+1;
    mnt=00;
  }
  if(hr==24){
    mnt=hr=00;
  }
  drawTime();
  while(hr==6&&mnt>=3&&mnt<5){
    drawHomeScreen();
    break;
  }
}


void drawHomeScreen() {
  
  //med name and dosage title
  tft.setCursor(5, 280);
  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.print("Name"); 

  //med name and dosage value
  tft.setCursor(5, 330);
  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.print("Aspirin"); 

  //med instruction title
  tft.setCursor(5, 390);
  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.print("Instruction   Dosage");

  //med instruction value
  tft.setCursor(5, 430);
  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.print("After Eating    1"); 
}

void drawTime(){
  tft.setRotation(0);     //vertical
  tft.fillScreen(BLACK);
  tft.setCursor(5,5);
  tft.setTextColor(BLUE);
  tft.setTextSize(3);
  tft.print(hr);
  tft.print(":");
  tft.print(mnt);
}
