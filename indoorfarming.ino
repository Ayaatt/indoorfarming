#include <DHT.h>
#include "RTClib.h"
// #include "uRTCLib.h"
#include <Wire.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoOTA.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)

#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Workshop Elka"
#define WIFI_PASSWORD "gapakekabel"
// const char *otaPassword = "cricket-ota";

// Insert Firebase project API Key
#define API_KEY "AIzaSyCq9s13ZGnSAC0ZW4SyjmvYTJT-32g7e6E"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://monitoringkandangjangkrik-default-rtdb.asia-southeast1.firebasedatabase.app"

#define DHT_PIN 32
#define DHT_TYPE DHT21
#define waterFlow 33

#define LAMPU 14
#define MISTMAKER 27
#define KIPAS_KECIL 25
#define KIPAS_BESAR 26

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// uRTCLib rtcData(0x57);
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
float suhuFuzzy[4];
float kelembapanFuzzy[4];

bool rulesKering = false;
bool rulesNormalLembap = false;
bool rulesSangatLembab = false;
bool rulesDingin = false;
bool rulesNormal = false;
bool rulesPanas = false;

float a, b;
float fanRule00, fanRule01, fanRule02, fanRule10, fanRule11, fanRule12, fanRule20, fanRule21, fanRule22, fanRule30, fanRule31, fanRule32;
float mistRule00, mistRule01, mistRule02, mistRule10, mistRule11, mistRule12, mistRule20, mistRule21, mistRule22, mistRule30, mistRule31, mistRule32;
float konjj;
float fanRule[3][3];
float mistRule[3][3];
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int intValue;
float floatValue;
bool signupOK = false;

const int analogMin = 0;    // Minimum analog reading
const int analogMax = 4095; // Maximum analog reading for ESP32
unsigned long previousMillis = 0;
const long interval = 2000; // Interval for switching display
bool displayTempHum = true;

void setup()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    // Inisialisasi OTA
    ArduinoOTA.setHostname("ESP32-CRICKET-OTA");
    // ArduinoOTA.setPassword(otaPassword);
    ArduinoOTA.begin();
    Serial.println("✅ OTA Siap, ESP32 menunggu pembaruan firmware...");

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("ok");
        signupOK = true;
    }
    else
    {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    Serial.begin(115200);
    pinMode(LAMPU, OUTPUT);
    pinMode(MISTMAKER, OUTPUT);
    pinMode(KIPAS_KECIL, OUTPUT);
    pinMode(KIPAS_BESAR, OUTPUT);

    // URTCLIB_WIRE.begin();

    //   RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
    // rtcData.set(3, 4, 20, 2, 8, 1, 24);
    // if (!rtc.begin())
    // {
    //     Serial.println("Couldn't find RTC");
    //     while (1)
    //         ;
    // }

    // if (rtc.lostPower())
    // {
    //     Serial.println("RTC lost power, let's set the time!");
    //     // When time needs to be set on a new device, or after a power loss, the
    //     // following line sets the RTC to the date & time this sketch was compiled
    //     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //     // This line sets the RTC with an explicit date & time, for example to set
    //     // January 21, 2014 at 3am you would call:
    //     // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    // }
    dht.begin();
    lcd.begin();     // Initialize the LCD
    lcd.backlight(); // Turn on the backlight
}

void loop()
{
    ArduinoOTA.handle();
    lcd.clear();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int analogWaterFlow = analogRead(waterFlow);
    int waterLevelPercentage = map(analogWaterFlow, analogMin, analogMax, 0, 100); // Map the analog reading to percentage
    Serial.print(waterLevelPercentage);
    Serial.println(" %");
    Serial.print(" ");
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t))
    {
        Serial.println(F("Failed to read from DHT sensor!"));
        h = 0;
        t = 0;
    }

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();
        if (Firebase.RTDB.setFloat(&fbdo, "suhu/", t))
        {
            Serial.println("PASSED");
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
        if (Firebase.RTDB.setFloat(&fbdo, "kelembapan/", h))
        {
            Serial.println("PASSED");
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
        if (Firebase.RTDB.setFloat(&fbdo, "water_level/", waterLevelPercentage))
        {
            Serial.println("PASSED");
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
        if (Firebase.RTDB.getBool(&fbdo, "fan/"))
        {

            intValue = fbdo.boolData();
            digitalWrite(KIPAS_BESAR, intValue);
            digitalWrite(KIPAS_KECIL, intValue);
            Serial.println("Fan: ");
            Serial.println(intValue);
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }

        if (Firebase.RTDB.getBool(&fbdo, "lamp"))
        {

            floatValue = fbdo.boolData();

            digitalWrite(LAMPU, floatValue);
            Serial.println("Lampu: ");
            Serial.println(floatValue);
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }

        if (Firebase.RTDB.getBool(&fbdo, "mist"))
        {

            floatValue = fbdo.boolData();
            digitalWrite(MISTMAKER, floatValue);
            Serial.println("Mist: ");
            Serial.println(floatValue);
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
    }

    // DateTime now = rtc.now();
    // Serial.print(now.year(), DEC);
    // Serial.print('/');
    // Serial.print(now.month(), DEC);
    // Serial.print('/');
    // Serial.print(now.day(), DEC);
    // Serial.print(" (");
    // Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    // Serial.print(") ");
    // Serial.print(now.hour(), DEC);
    // Serial.print(':');
    // Serial.print(now.minute(), DEC);
    // Serial.print(':');
    // Serial.print(now.second(), DEC);
    // Serial.print(" ");

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

        char tempStr[6];
        char humStr[6];
        char waterLevelStr[6];
        dtostrf(t, 4, 1, tempStr);
        dtostrf(h, 4, 1, humStr);
        dtostrf(waterLevelPercentage, 4, 1, waterLevelStr);

        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis;
            displayTempHum = !displayTempHum; // Toggle the display state
        }

        lcd.clear();
        if (displayTempHum)
        {
            lcd.setCursor(0, 0);
            lcd.print("T:");
            lcd.print(tempStr);
            lcd.print(" C");
            lcd.setCursor(0, 1);
            lcd.print("H:");
            lcd.print(humStr);
            lcd.print(" %");
        }
        else
        {
            lcd.setCursor(0, 0);
            lcd.print("Water Level:");
            lcd.setCursor(0, 1);
            lcd.print(waterLevelStr);
            lcd.print(" %");
        }
    }
    delay(2000);

    // if (h > 0 && t > 0)
    // {
    //     fuzzifikasiSuhu(t);
    //     fuzzifikasiKelembapan(h);
    //     rulesSugenoFan();
    //     rulesSugenoMist();
    //     float pwmFan = defuzifikasiFan();
    //     float pwmMist = defuzifikasiMist();
    //     Serial.print("Fan: ");
    //     Serial.println(pwmFan);
    //     Serial.print("Mist: ");
    //     Serial.println(pwmMist);
    //     if (pwmFan > 0)
    //     {
    //         digitalWrite(KIPAS_BESAR, HIGH);
    //         digitalWrite(KIPAS_KECIL, HIGH);
    //     }
    //     else
    //     {
    //         digitalWrite(KIPAS_BESAR, LOW);
    //         digitalWrite(KIPAS_KECIL, LOW);
    //     }
    //     if (pwmMist > 0)
    //     {
    //         // ganti mistmaker
    //         digitalWrite(LAMPU, HIGH);
    //     }
    //     else
    //     {
    //         digitalWrite(LAMPU, LOW);
    //     }
    // }
}

void fuzzifikasiSuhu(float suhu)
{
    // dingin
    if (suhu >= 0 && suhu <= 25)
    {
        suhuFuzzy[0] = 1;
        rulesDingin = true;
    }
    else if (suhu > 25 && suhu < 27.5)
    {
        suhuFuzzy[0] = (-1 * (suhu - b)) / (b - a);
        ;

        rulesDingin = true;
    }
    else
    {
        suhuFuzzy[0] = 0;
        rulesDingin = false;
    }

    // normal
    if (suhu >= 20 && suhu < 25)
    {
        a = 20;
        b = 25;
        suhuFuzzy[1] = (suhu - a) / (b - a);
        rulesNormal = true;
        // valueNormal = suhuFuzzy[1];
    }
    else if (suhu >= 25 && suhu <= 32.5)
    {
        suhuFuzzy[1] = 1;
        rulesNormal = true;
        // valueNormal = suhuFuzzy[1];
    }
    else if (suhu > 32.5 && suhu < 35)
    {
        a = 32.5;
        b = 35;
        suhuFuzzy[1] = (-1 * (suhu - b)) / (b - a);
        rulesNormal = true;
    }

    else
    {
        suhuFuzzy[1] = 0;
    }
    // panas
    if (suhu > 30 && suhu <= 32.5)
    {
        a = 30;
        b = 32.5;
        suhuFuzzy[2] = (suhu - a) / (b - a);
        rulesPanas = true;

        // valuePanas = suhuFuzzy[2];
    }
    else if (suhu >= 32.5 && suhu < 50)
    {
        suhuFuzzy[2] = 1;
        rulesPanas = true;
    }
    else
    {
        suhuFuzzy[2] = 0;
    }
}

void fuzzifikasiKelembapan(float kelembapan)
{
    // sangat kering
    if (kelembapan > 0 && kelembapan <= 67.5)
    {
        kelembapanFuzzy[0] = 1;
        rulesKering = true;
    }
    else if (kelembapan > 67.5 && kelembapan < 70)
    {
        a = 67.5;
        b = 70;
        kelembapanFuzzy[0] = (-1 * (kelembapan - b)) / (b - a);
        rulesKering = true;
    }
    else
    {
        kelembapanFuzzy[0] = 0;
    }
    // kering
    if (kelembapan > 65 && kelembapan < 67.5)
    {
        a = 65;
        b = 67.5;
        kelembapanFuzzy[1] = (kelembapan - a) / (b - a);
        rulesNormalLembap = true;
    }
    else if (kelembapan >= 67.5 && kelembapan <= 75)
    {
        kelembapanFuzzy[1] = 1;
        rulesNormalLembap = true;
    }
    else if (kelembapan > 75 && kelembapan <= 80)
    {
        a = 75;
        b = 80;
        kelembapanFuzzy[1] = (-1 * (kelembapan - b)) / (b - a);
        rulesNormalLembap = true;
    }
    else
    {
        kelembapanFuzzy[1] = 0;
    }

    // normal

    if (kelembapan > 75 && kelembapan < 80)
    {
        a = 75;
        b = 80;
        kelembapanFuzzy[2] = (kelembapan - a) / (b - a);
        rulesSangatLembab = true;
    }
    else if (kelembapan >= 80 && kelembapan <= 100)
    {
        kelembapanFuzzy[2] = 1;
        rulesSangatLembab = true;
    }

    else
    {
        kelembapanFuzzy[2] = 0;
    }
}

void rulesSugenoFan()
{
    if (rulesDingin || rulesNormal || rulesPanas || rulesKering || rulesNormalLembap || rulesSangatLembab)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                konjj = max(suhuFuzzy[i], kelembapanFuzzy[j]);
                fanRule[i][j] = konjj;
            }
        }
    }
    fanRule00 = fanRule[0][0]; // dingin - kering = OFF
    fanRule01 = fanRule[0][1]; // dingin - kering = OFF
    fanRule02 = fanRule[0][2]; // dingin - normal = OFF

    fanRule10 = fanRule[1][0]; // normal - sangat kering = ON
    fanRule11 = fanRule[1][1]; // normal - kering = OFF
    fanRule12 = fanRule[1][2]; // normal - normal = OFF

    fanRule20 = fanRule[2][0]; // panas - sangat kering = OFF
    fanRule21 = fanRule[2][1]; // panas - kering = ON
    fanRule22 = fanRule[2][2]; // panas - normal = ON
}

float defuzifikasiFan()
{
    float toneLambat = 0.0f;
    float toneNormal = 1.0f; // Keep as float for consistent calculation
    float pwmFan = 0.0f;     // Use float to prevent truncation
    float maximumLambat[3] = {fanRule10, fanRule21, fanRule22};
    float maximumNormal[6] = {fanRule00, fanRule01, fanRule02, fanRule11, fanRule12, fanRule20};

    for (int i = 0; i < sizeof(maximumLambat) / sizeof(*maximumLambat) - 1; i++)
    {
        pwmFan += maximumLambat[i] * toneLambat;
    }
    for (int i = 0; i < sizeof(maximumNormal) / sizeof(*maximumNormal) - 1; i++)
    {
        pwmFan += maximumNormal[i] * toneNormal;
    }

    float alphaPredic = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            alphaPredic += fanRule[i][j];
        }
    }
    pwmFan = pwmFan / alphaPredic;

    return pwmFan;
}

void rulesSugenoMist()
{
    if (rulesDingin || rulesNormal || rulesPanas || rulesKering || rulesNormalLembap || rulesSangatLembab)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                konjj = max(suhuFuzzy[i], kelembapanFuzzy[j]);
                mistRule[i][j] = konjj;
            }
        }
    }

    mistRule00 = mistRule[0][0]; // dingin - kering = on
    mistRule01 = mistRule[0][1]; // dingin - lembap = off
    mistRule02 = mistRule[0][2]; // dingin - s_lembap = off

    mistRule10 = mistRule[1][0]; // normal - kering = on
    mistRule11 = mistRule[1][1]; // normal - lembap = on
    mistRule12 = mistRule[1][2]; // normal - s_lembap = off

    mistRule20 = mistRule[2][0]; // panas- kering = off
    mistRule21 = mistRule[2][1]; // panas - lembap = on
    mistRule22 = mistRule[2][2]; // panas - s_lembap = on
}

float defuzifikasiMist()
{
    float toneRendah = 0.0f;
    float toneNormal = 1.0f;                                                   // Keep as float for consistent calculation
    float pwmMist = 0.0f;                                                      // Use float to prevent truncation
    float maximumRendah[4] = {mistRule01, mistRule02, mistRule12, mistRule20}; // off
    float maximumNormalMist[7] = {mistRule00, mistRule10, mistRule11, mistRule21, mistRule22};
    for (int i = 0; i <= sizeof(maximumRendah) / sizeof(*maximumRendah) - 1; i++)
    {

        pwmMist += maximumRendah[i] * toneRendah;
    }
    for (int i = 0; i <= sizeof(maximumNormalMist) / sizeof(*maximumNormalMist) - 1; i++)
    {

        pwmMist += maximumNormalMist[i] * toneNormal;
    }

    float alphaPredic = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            alphaPredic += mistRule[i][j];
        }
    }
    if (pwmMist >= 1)
    {
        /* code */ pwmMist = 1;
    }

    return pwmMist;
}
