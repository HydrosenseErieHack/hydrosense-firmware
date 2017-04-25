#include <Arduino.h>
#include "OneWire.h"
#include "DallasTemperature.h"

#define ONE_WIRE_PIN 11

/* start a one wire instance on the desired pin */
OneWire oneWire(ONE_WIRE_PIN);

/* initialize the temperature library using one wire */
DallasTemperature tempSensor(&oneWire);

void setup( void )
{
  /* init console logging */
  Serial.begin(9600);
  while(!Serial);

  /* init the temperature sensor */
  tempSensor.begin();
}

void loop( void )
{
  tempSensor.requestTemperatures();
  Serial.println(tempSensor.getTempCByIndex(0));

  delay(2000);
}
