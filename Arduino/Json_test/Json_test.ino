#include <SD.h>
const int chipSelect = 4;
void setup()
{
 Serial.begin(115200);
 Serial.print("Initializing SD card...");
 // make sure that the default chip select pin is set to
 // output, even if you don't use it:
 pinMode(10, OUTPUT);
 // see if the card is present and can be initialized:
 if (!SD.begin(chipSelect)) {
 Serial.println("Card failed, or not present");
 // don't do anything more:
 return;
 }
 Serial.println("card initialized.");
}
void loop()
{
 String dataString ="{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
 File dataFile = SD.open("datalog1.jso", FILE_READ);
 if (dataFile) {
   /*dataFile.println(dataString);
   dataFile.close();
   Serial.println(data);*/
   while (dataFile.available()) {
    Serial.print((char)dataFile.read());
  }
  Serial.println();
  dataFile.close();
 }
 else {
  Serial.println("error opening datalog.json");
 }
}
