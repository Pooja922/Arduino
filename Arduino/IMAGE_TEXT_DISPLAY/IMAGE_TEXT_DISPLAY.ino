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

#if defined(ESP32)
#define SD_CS     5
#else
#define SD_CS     10
#endif
#define NAMEMATCH "pic"        // "" matches any names
#define PALETTEDEPTH   0     // do not support Palette modes

char namebuf[32] = "/";   //BMP files in root directory

File root;
int pathlen;
int hr,mnt;
void setup()
{
  tft.reset();
  tft.begin(0x9486);
  SD.begin(SD_CS);
  root = SD.open(namebuf);
  pathlen = strlen(namebuf); 
  hr=6;
  mnt=2;
  drawTime();
}

void loop()
{
  drawImage();
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
                delay(60000);
                mnt=mnt+1;
                if(mnt==60){
                  hr=hr+1;
                  mnt=0;
                }
                if(hr==24){
                  mnt=hr=0;
                }
                break;
              default:
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
  uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
  uint8_t bitmask, bitshift;
  boolean flip = true;        // BMP is stored bottom-to-top
  int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
  uint32_t pos;               // seek position
  boolean is565 = false;      //

  uint16_t bmpID;
  uint16_t n;                 // blocks read
  uint8_t ret;

  if ((x >= tft.width()) || (y >= tft.height()))
    return 1;               // off screen

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
  else if (n != 1) ret = 3;   // too many planes
  else if (pos != 0 && pos != 3) ret = 4; // format: 0 = uncompressed, 3 = 565
  else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH) ret = 5; // palette
  else {
    bool first = true;
    is565 = (pos == 3);               // ?already in 16-bit format
    // BMP rows are padded (if needed) to 4-byte boundary
    rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
    if (bmpHeight < 0) {              // If negative, image is in top-down order.
      bmpHeight = -bmpHeight;
      flip = false;
    }

    w = bmpWidth;
    h = bmpHeight;
    if ((x + w) >= tft.width())       // Crop area to be loaded
      w = tft.width() - x;
    if ((y + h) >= tft.height())      //
      h = tft.height() - y;

    if (bmpDepth <= PALETTEDEPTH) {   // these modes have separate palette
      //bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
      bmpFile.seek(bmpImageoffset - (4 << bmpDepth)); //54 for regular, diff for colorsimportant
      bitmask = 0xFF;
      if (bmpDepth < 8)
        bitmask >>= bmpDepth;
      bitshift = 8 - bmpDepth;
      n = 1 << bmpDepth;
      lcdbufsiz -= n;
      palette = lcdbuffer + lcdbufsiz;
      for (col = 0; col < n; col++) {
        pos = read32(bmpFile);    //map palette to 5-6-5
        palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
      }
    }

    // Set TFT address window to clipped image bounds
    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    for (row = 0; row < h; row++) {
      uint8_t r, g, b, *sdptr;
      int lcdidx, lcdleft;
      if (flip)   // Bitmap is stored bottom-to-top order (normal BMP)
        pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
      else        // Bitmap is stored top-to-bottom
        pos = bmpImageoffset + row * rowSize;
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
            case 16:
              b = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              if (is565)
                color = (r << 8) | (b);
              else
                color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F);
              break;
            case 1:
            case 4:
            case 8:
              if (r == 0)
                b = sdbuffer[buffidx++], r = 8;
              color = palette[(b >> bitshift) & bitmask];
              r -= bmpDepth;
              b <<= bmpDepth;
              break;
          }
          lcdbuffer[lcdidx] = color;

        }
        tft.pushColors(lcdbuffer, lcdidx, first);
        first = false;
        col += lcdidx;
      }           // end cols
    }               // end rows
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
    drawTime();
        tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
        ret = 0;        // good render
    }
    bmpFile.close();
    return (ret);
}
void drawTime(){
  tft.setTextSize(1);
  tft.setCursor(5,470);
  tft.setTextColor(CYAN);
  tft.print(hr);
  tft.print(":");
  tft.print(mnt);
}
