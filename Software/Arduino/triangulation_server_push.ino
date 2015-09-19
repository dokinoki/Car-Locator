#include <SoftwareSerial.h>
#include <Adafruit_FONA.h>

#define FONA_RX 2
#define FONA_TX 10
#define FONA_RST 4
#define AIO_URL "http://letsdofunshit.today/prius/status/insert"

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

void setup() {

  // Serial init
  Serial.begin(115200);
  Serial.println(F("Initializing....(May take a few seconds)"));

  // SoftwareSerial init
  fonaSS.begin(4800);

  // bail if we can't connect to the FONA
  while (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA, retrying connection..."));
    delay(10000);
  }

  fona.enableGPRS(false);
  while(!fona.enableGPRS(true)) {
    Serial.println(F("gprs init failed... trying again"));
    delay(2000);
  }
  
  Serial.println(F("gprs enabled"));

}

void loop() {
   // check for GSMLOC (requires GPRS)
   float lat;
   float lon;
   
   // TODO: Add gps and temperature sensor and replace these values 
   int pre = 1;
   float tem = 18.7;
   
   if (fona.getGSMLoc(&lon, &lat)){
     sendData(lat, lon, pre, tem);
   } else {
     Serial.println(F("Failed!"));
   }
   
   delay(15000);
}

bool sendData(float lat, float lon, int pre, float tem) {

  // init return values
  uint16_t statuscode;
  int16_t length;
  
  //Sprintf does not work with floats in arduino (yay...)
  char charLat[8]; // buffer for latitude
  dtostrf(lat, 4, 2, charLat); // Min. 6 chars wide incl. decimal point, 2 digits right of decimal
  
  char charLon[8]; // buffer for longitude
  dtostrf(lon, 4, 2, charLon); // Min. 6 chars wide incl. decimal point, 2 digits right of decimal
  
  char charTem[6]; // buffer for temperature
  dtostrf(tem, 4, 2, charTem); // Min. 6 chars wide incl. decimal point, 2 digits right of decimal
  
  Serial.println(charLat);
  Serial.println(charLon);
  Serial.println(charTem);

  // allow for ~60 character feed name or key
  char url[150];
  // allow for ~60 character value
  char data[80];

  // pull together the post body
  sprintf(data, "{\"lat\": \"%s\", \"lon\": \"%s\", \"pre\": \"%i\", \"tem\": \"%s\"}", charLat, charLon, pre, charTem);

  // print urls for debugging
  Serial.println(AIO_URL);
  Serial.println(data);

  // send post
  if(! fona.HTTP_POST_start(AIO_URL, F("application/json"), (uint8_t *)data, strlen(data), &statuscode, (uint16_t *)&length))
    return false;

  fona.HTTP_POST_end();
  
  delay(10000);

  // should return a HTTP 201
  return statuscode == 201;

}
