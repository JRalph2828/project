const int ledPin=13;
int blinkRate=0;

void setup() 
{
  Serial.begin(9600);
  pinMode(ledPin,OUTPUT);
}

int number = 0;

void loop() 
{
  if(Serial.available()){
    String buffer = Serial.readString();
    blinkRate = buffer.toInt();

    Serial.print("delay : ");
    Serial.print(blinkRate);
    Serial.println("ms");

  }
  blink();
}

void blink()
{
  digitalWrite(ledPin,HIGH);
  delay(blinkRate);
  digitalWrite(ledPin,LOW);
  delay(blinkRate);
}

