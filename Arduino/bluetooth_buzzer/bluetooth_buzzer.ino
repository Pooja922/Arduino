void setup() {
  // put your setup code here, to run once:
  pinMode(13,OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if(Serial.available()>0)
  {
    int data=Serial.read();
    Serial.println(data);
    switch(data)
    {
      case 49:
      digitalWrite(13,HIGH);
      break;
      case 50:
      digitalWrite(13,LOW);
      break;
    }
  }
}
