#include <BBQ10Keyboard.h>

#define CARD_DET 1

BBQ10Keyboard keyboard;
int prevValue = LOW;

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  keyboard.begin();
  keyboard.pinMode(CARD_DET, INPUT);
  keyboard.digitalWrite(CARD_DET, HIGH); // Enable pull-up
}

void loop()
{
  const int value = keyboard.digitalRead(CARD_DET);
  if (value == prevValue)
    return;

  Serial.printf("Card Detect: %d \r\n", value);

  prevValue = value;
}
