#include <M5Stack.h>
#include <TinyGPS++.h>

HardwareSerial GpsSerial(2);

TinyGPSPlus gps;            // The TinyGPS++ object

int baudrate = 9600;


/**
 * @brief 
 * 
 * @param ms 
 */
static void serialProc(unsigned long ms)
{
    unsigned long start = millis();
    do 
    {
        /// Read.  (gps -> pc)
        while (GpsSerial.available() > 0)
        {
            int c = GpsSerial.read();
            Serial.write(c);
            gps.encode(c);
        }

        /// Write.  (pc -> gps)
        while(Serial.available() > 0)
        {
            int c = Serial.read();
            GpsSerial.write(c);
        }
        
    } while (millis() - start < ms);
    
    M5.Lcd.clear();
}

void displayInfo()
{
    M5.Lcd.setCursor(0, 40, 4);
    M5.Lcd.print(F("Latitude:    ")); 
    
    if (gps.location.isValid())
    {
        M5.Lcd.print(gps.location.lat(), 6);
    }
    else
    {
        M5.Lcd.print(F("INVALID"));
    }

    M5.Lcd.println();
    M5.Lcd.print(F("Longitude:    ")); 
    
    if (gps.location.isValid())
    {
        M5.Lcd.print(gps.location.lng(), 6);   
    }
    else
    {
        M5.Lcd.print(F("INVALID"));
    }

    M5.Lcd.println();
    M5.Lcd.print(F("Altitude:    ")); 
    if (gps.altitude.isValid())
    {
        M5.Lcd.print(gps.altitude.meters());
    }
    else
    {
        M5.Lcd.print(F("INVALID"));
    }

    M5.Lcd.println();
    M5.Lcd.print(F("Satellites:    "));
    if (gps.satellites.isValid())
    {
        M5.Lcd.print(gps.satellites.value());
    }
    else
    {
        M5.Lcd.print(F("INVALID"));
    }

    M5.Lcd.println();
    M5.Lcd.print(F("Date: "));
    
    if (gps.date.isValid())
    {
        M5.Lcd.print(gps.date.month());
        M5.Lcd.print(F("/"));
        M5.Lcd.print(gps.date.day());
        M5.Lcd.print(F("/"));
        M5.Lcd.print(gps.date.year());
    }
    else
    {
        M5.Lcd.print(F("INVALID"));
    }

    M5.Lcd.println();
    M5.Lcd.print(F("Time: "));

    if (gps.time.isValid())
    {
        if (gps.time.hour() < 10) M5.Lcd.print(F("0"));
        M5.Lcd.print(gps.time.hour());
        M5.Lcd.print(F(":"));
        if (gps.time.minute() < 10) M5.Lcd.print(F("0"));
        M5.Lcd.print(gps.time.minute());
        M5.Lcd.print(F(":"));
        if (gps.time.second() < 10) M5.Lcd.print(F("0"));
        M5.Lcd.print(gps.time.second());
        M5.Lcd.print(F("."));
        if (gps.time.centisecond() < 10) M5.Lcd.print(F("0"));
        M5.Lcd.print(gps.time.centisecond());
    }
    else
    {
        M5.Lcd.print(F("INVALID"));
    }
}

/**
 * @brief entry point.
 * 
 */
void setup()
{
    M5.begin();
    M5.Speaker.begin();
    M5.Speaker.mute();
    
    GpsSerial.begin(baudrate);
    Serial.begin(baudrate);
    
    M5.Lcd.setTextColor(GREEN, BLACK);
}

/**
 * @brief このloopが延々繰り返し呼ばれる。
 * 
 */
void loop()
{
    displayInfo();
    serialProc(1000);
}
