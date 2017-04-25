#include <Arduino.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include "fona/fona.h"

#define ONE_WIRE_PIN 11

/* start a one wire instance on the desired pin */
OneWire oneWire(ONE_WIRE_PIN);

/* initialize the temperature library using one wire */
DallasTemperature tempSensor(&oneWire);

float longitude;
float latitude;

void setup( void )
{
  /* init console logging */
  Serial.begin(9600);
  while(!Serial);

  /* init the temperature sensor */
  tempSensor.begin();

  /* init the FONA module */
  FONA_setup();
}

void loop( void )
{
  tempSensor.requestTemperatures();
  Serial.println(tempSensor.getTempCByIndex(0));

  Get_GPS_Data(longitude, latitude);
  Serial.println(longitude);
  Serial.println(latitude);

  if (longitude  != 0.0 || latitude != 0.0)
  {
    Serial.println("Sending");
    if( Send_Sensor_Data_To_LTE(0, 0, tempSensor.getTempCByIndex(0), latitude, longitude, 1) )
    {
        while(1);
    }
  }

  delay(2000);
}
