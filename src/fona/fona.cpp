#include <SoftwareSerial.h>

#include "Adafruit_FONA.h"
#include "fona.h"

//Defines forLTE Chip  need this for current arduino to work.
#define FONA_RX 2
#define FONA_TX 10
#define FONA_RST 4

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

void Turn_On_GPS()
{
  if (!fona.enableGPS(true))
      Serial.println(F("Failed to turn on"));
}

void LTE_SETUP()
{
 // Serial.println(F("FONA basic test"));
  //Serial.println(F("Initializing....(May take 3 seconds)"));

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial))
  {
    //Serial.println(F("Couldn't find FONA"));
    while (1);
  }
}

void FONA_setup()
{
  LTE_SETUP();
  delay(1000);
  Turn_On_GPS();
  delay(1000);
}

void flushSerial()
{
  while (Serial.available())
    Serial.read();
}

//Data here is shown below alag_con is alage concenration, turb is turbulance lat is latatude lon is lonintude.
bool Send_Sensor_Data_To_LTE(int alag_con, int turb, float Water_Temp, float lat, float lon, int sense_num) {
  static bool already_true = false;

  if (already_true == false) {

    if (fona.enableGPRS(true)) {
      // read website URL
      uint16_t statuscode;
      int16_t length;
      char url[250];

      flushSerial();
      Serial.println(F("NOTE: in beta! Use small webpages to read!"));
      Serial.println(F("URL to read (e.g. www.adafruit.com/testwifi/index.html):"));
      String UR_L = "http://74.141.20.92/ErieHack/EriehackLTE.php?con=";
      UR_L = UR_L + alag_con + "&temp=" + Water_Temp + "&lat=" + lat + "&lon=" + lon + "&turb=" + turb + "&Sense_Num=" + sense_num;
      UR_L.toCharArray(url, 180);
      Serial.println("The URL IS:");
      Serial.println(url);

      Serial.println(F("****"));
      if (!fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
        Serial.println("Failed! http");
      }
      already_true = true;
      while (length > 0) {
        while (fona.available()) {
          char c = fona.read();

          // Serial.write is too slow, we'll write directly to Serial register!
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
          loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
          UDR0 = c;
#else
          Serial.write(c);
#endif
          length--;
          if (! length) break;
        }
      }
      Serial.println(F("\n****"));
      fona.HTTP_GET_end();
    }
    else {
      Serial.println("Cant connect!");
      return false;
    }
  }
  else {
    // read website URL
    uint16_t statuscode;
    int16_t length;
    char url[180];

    flushSerial();
    Serial.println(F("NOTE: in beta! Use small webpages to read!"));
    Serial.println(F("URL to read (e.g. www.adafruit.com/testwifi/index.html):"));
    String UR_L = "http://74.141.20.92/ErieHack/EriehackLTE.php?con=";
    UR_L = UR_L + alag_con + "&temp=" + Water_Temp + "&lat=" + lat + "&lon=" + lon + "&turb=" + turb + "&Sense_Num=" + sense_num;;
    UR_L.toCharArray(url, 180);
    Serial.println("The URL IS:");
    Serial.println(url);

    Serial.println(F("****"));
    if (!fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
      Serial.println("Failed!");
    }
    while (length > 0) {
      while (fona.available()) {
        char c = fona.read();

        // Serial.write is too slow, we'll write directly to Serial register!
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
        loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
        UDR0 = c;
#else
        Serial.write(c);
#endif
        length--;
        if (! length) break;
      }
    }
    Serial.println(F("\n****"));
    fona.HTTP_GET_end();
  }
  return true; 
}

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout)
{
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true)
  {
    if (buffidx > maxbuff)
    {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available())
    {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}

void Get_GPS_Data( float& longitude, float& latitude)
{
    char lon[30];
    char lati[30];
    bool got_lat = false;
    bool got_lon = false;
    int count = 0;
    int inc = 0;
    int char_inc = 0;
    // check for GPS location
    char gpsdata[120];
    fona.getGPS(0, gpsdata, 120);
    Serial.println(F("Reply in format: mode,fixstatus,utctime(yyyymmddHHMMSS),latitude,longitude,altitude,speed,course,fixmode,reserved1,HDOP,PDOP,VDOP,reserved2,view_satellites,used_satellites,reserved3,C/N0max,HPA,VPA"));
    Serial.println(gpsdata);
    while (count < 3)
    {
      if (gpsdata[inc] == ',')
      {
        count = count + 1;
      }
      inc++;
    }
    while (got_lat == false)
    {
      lati[char_inc] = gpsdata[inc];
      char_inc++;
      inc++;
      if (gpsdata[inc] == ',')
      {
        got_lat = true;
        lati[char_inc] = 0;
      }
    }
    char_inc = 0;
    inc++;
    //Finished Lat now get lon
    while (got_lon == false)
    {
      lon[char_inc] = gpsdata[inc];
      inc++;
      char_inc++;
      if (gpsdata[inc] == ',')
      {
        lon[char_inc] = 0;
        got_lon = true;
      }
    }
    latitude = atof(lati);
    longitude = atof(lon);
}
