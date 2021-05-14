#include<TFT.h>
#include<SPI.h>
#define cs   10
#define dc   9
#define rst  8
TFT TFTscreen = TFT(cs, dc, rst);
void setup() {
  // begin serial communication
  Serial.begin(9600);
  // initialize the display
  TFTscreen.begin();
  // set the background to white
  TFTscreen.background(255, 255, 255);
}
void loop() {
  // read the values from your sensors and scale them to 0-255
  int redVal = map(analogRead(A0), 0, 1023, 0, 255);
  int greenVal = map(analogRead(A1), 0, 1023, 0, 255);
  int blueVal = map(analogRead(A2), 0, 1023, 0, 255);
  // draw the background based on the mapped values
  TFTscreen.background(redVal, greenVal, blueVal);
  // send the values to the serial monitor
  Serial.print("background(");
  Serial.print(redVal);
  Serial.print(" , ");
  Serial.print(greenVal);
  Serial.print(" , ");
  Serial.print(blueVal);
  Serial.println(")");
  // wait for a moment
  delay(33);
}
