#include <DHT.h>
#include "RTClib.h"
// #include "uRTCLib.h"
#include <Wire.h>

#define DHT_PIN 32
#define DHT_TYPE DHT21
#define waterFlow 33

DHT dht(DHT_PIN, DHT_TYPE);

// uRTCLib rtcData(0x57);
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup()
{
    Serial.begin(115200);
    // URTCLIB_WIRE.begin();

    //   RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
    // rtcData.set(3, 4, 20, 2, 8, 1, 24);
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        while (1)
            ;
    }

    if (rtc.lostPower())
    {
        Serial.println("RTC lost power, let's set the time!");
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
    dht.begin();
}

void loop()
{
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    DateTime now = rtc.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.print(" ");

    int analogWaterFlow = analogRead(waterFlow);
    Serial.print("Analog Water Flow: ");
    Serial.print(analogWaterFlow);
    Serial.print(" ");

    // int tahun = rtcData.year();
    // int bulan = rtcData.month();
    // int tanggal = rtcData.day();
    // int jam = rtcData.hour();
    // int menit = rtcData.minute();
    // int detik = rtcData.second();
    // Serial.print("Tanggal: ");
    // Serial.print(tanggal);
    // Serial.print("/");
    // Serial.print(bulan);
    // Serial.print("/");
    // Serial.print(tahun);
    // Serial.print(" ");
    // Serial.print(jam);
    // Serial.print(":");
    // Serial.print(menit);
    // Serial.print(":");
    // Serial.print(detik);
    // Serial.print(" ");

    if (isnan(t) || isnan(h))
    {
        Serial.println("Failed to read from DHT");
    }
    else
    {
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" *C");
    }
    delay(2000);
}
