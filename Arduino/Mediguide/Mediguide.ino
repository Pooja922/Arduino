#include <SPI.h>            // f.k. for Arduino-1.5.2
#include <SD.h>             // Use the official SD library on hardware pins
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans18pt7b.h>
#include <MCUFRIEND_kbv.h>

#define BLACK   0x0000
#define CYAN    0x07FF
#define DARKCYAN 0x03EF /* 0, 128, 128 */
MCUFRIEND_kbv tft;

#define SD_CS     10
#define NAMEMATCH "pic"        // "" matches any names
char namebuf[32] = "/";   //BMP files in root directory

File root;
int pathlen;
int hr,mnt;
void setup()
{
  tft.reset();
  tft.begin(0x9486);
  Serial.begin(9600);
  SD.begin(SD_CS);
  root = SD.open(namebuf);
  pathlen = strlen(namebuf); 
  hr=6;
  mnt=29;
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
  while(hr==6&&mnt>=30&&mnt<31){
    drawImage();
  }
}
void drawImage(){
  char *nm = namebuf + pathlen;
  File f = root.openNextFile();
  uint8_t ret;
  uint32_t start;
  if (f != NULL)
  {
      #ifdef USE_SDFAT
          f.getName(nm, 32 - pathlen);
      #else
          strcpy(nm, (char *)f.name());
      #endif
          f.close();
          strlwr(nm);
          if (strstr(nm, ".bmp") != NULL && strstr(nm, NAMEMATCH) != NULL) {
            tft.fillScreen(0);
            start = millis();
            ret = showBMP(namebuf, 5, 5);
            switch (ret) {
              case 0:
              delay(55000);
              mnt=mnt+1;
                break;
            }
          }
  }
  else root.rewindDirectory();
}
#define BMPIMAGEOFFSET 54

#define BUFFPIXEL      20

uint16_t read16(File& f) {
  uint16_t result;         // read little-endian
  f.read((uint8_t*)&result, sizeof(result));
  return result;
}

uint32_t read32(File& f) {
  uint32_t result;
  f.read((uint8_t*)&result, sizeof(result));
  return result;
}

uint8_t showBMP(char *nm, int x, int y)
{
  File bmpFile;
  int bmpWidth, bmpHeight;    // W+H in pixels
  uint8_t bmpDepth;           // Bit depth (currently must be 24, 16, 8, 4, 1)
  uint32_t bmpImageoffset;    // Start of image data in file
  uint32_t rowSize;           // Not always = bmpWidth; may have padding
  uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
  uint8_t sdbuffer1[]={1,40,1,32,0,16,74,70,73,70,0,1,1,0,0,1,0,1,0,0,1,37};
  uint16_t lcdbuffer[(1 << 0) + BUFFPIXEL], *palette = NULL;
  uint8_t bitmask, bitshift;
  boolean flip = true;        // BMP is stored bottom-to-top
  int w, h, row, col, lcdbufsiz = (1 << 0) + BUFFPIXEL, buffidx;
  uint32_t pos;               // seek position
  boolean is565 = false;      //

  uint16_t bmpID;
  uint16_t n;                 // blocks read
  uint8_t ret;

  bmpFile = SD.open(nm);      // Parse BMP header
  bmpID = read16(bmpFile);    // BMP signature
  (void) read32(bmpFile);     // Read & ignore file size
  (void) read32(bmpFile);     // Read & ignore creator bytes
  bmpImageoffset = read32(bmpFile);       // Start of image data
  (void) read32(bmpFile);     // Read & ignore DIB header size
  bmpWidth = read32(bmpFile);
  bmpHeight = read32(bmpFile);
  n = read16(bmpFile);        // # planes -- must be '1'
  bmpDepth = read16(bmpFile); // bits per pixel
  pos = read32(bmpFile);      // format
  if (bmpID != 0x4D42) ret = 2; // bad ID
  else {
    bool first = true;
    is565 = (pos == 3); 
    rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
    w = bmpWidth;
    h = bmpHeight;
    if ((x + w) >= tft.width())       // Crop area to be loaded
      w = tft.width() - x;
    if ((y + h) >= tft.height())      
      h = tft.height() - y;

    // Set TFT address window to clipped image bounds
    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    for (row = 0; row < h; row++) {
      uint8_t r, g, b, *sdptr;
      int lcdidx, lcdleft;
      pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
      if (bmpFile.position() != pos) { // Need seek?
        bmpFile.seek(pos);
        buffidx = sizeof(sdbuffer); // Force buffer reload
      }
      for (col = 0; col < w; ) {  //pixels in row
        lcdleft = w - col;
        if (lcdleft > lcdbufsiz) lcdleft = lcdbufsiz;
        for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) { // buffer at a time
          uint16_t color;
          // Time to read more pixel data?
          if (buffidx >= sizeof(sdbuffer)) { // Indeed
            bmpFile.read(sdbuffer, sizeof(sdbuffer));
            buffidx = 0; // Set index to beginning
            r = 0;
          }
          switch (bmpDepth) {          // Convert pixel from BMP to TFT format
            case 24:
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              color = tft.color565(r, g, b);
              break;
          }
          lcdbuffer[lcdidx] = color;
        }
        tft.pushColors(lcdbuffer, lcdidx, first);
        first = false;
        col += lcdidx;
      }           // end cols
    }               // end rows
        drawText();
        
    }
    bmpFile.close();
    return (0);
}
void drawText(){
  tft.setCursor(5, 100);
    tft.setTextColor(CYAN);
    tft.setTextSize(1);
    tft.setFont(&FreeSans18pt7b);
    tft.print("Name"); 
    tft.setCursor(5, 150);
    tft.setTextColor(DARKCYAN);
    tft.print("Aspirin");
    tft.setCursor(5, 210);
    tft.setTextColor(CYAN);
    tft.print("Instruction   Dosage");
    tft.setCursor(5, 260);
    tft.setTextColor(DARKCYAN);
    tft.print("After Eating    1"); 
}
void drawTime(){
  tft.fillScreen(BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSans18pt7b);
  tft.setCursor(5,30);
  tft.setTextColor(CYAN);
  tft.print(hr);
  tft.print(":");
  tft.print(mnt);
}
