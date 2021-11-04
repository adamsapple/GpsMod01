#include <M5Stack.h>
#include <TinyGPS++.h>
#include <ButtonManager.h>


HardwareSerial GpsSerial(2);

TinyGPSPlus gps;            // The TinyGPS++ object

const int baudrate  = 9600;
const int fps       = 1;

// DEBUG
ButtonManager buttonManager;
static const int num_buttons = static_cast<std::underlying_type<ButtonType>::type>(ButtonType::Num_Buttons);
int colors[num_buttons];
std::uint32_t btns_millis[num_buttons];

/**
 * @brief gpsモジュールとの通信処理.
 * 
 */
static void serialProc()
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
}

/**
 * @brief 位置情報の描画.
 * 
 */
void displayInfo()
{
    M5.Lcd.clear();
    M5.Lcd.setTextColor(GREEN, BLACK);

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
 * @brief ボタンの状態を描画
 * 
 */
void drawButtonsStatus()
{
    {
		std::uint32_t now = millis();

		for (int i = 0; i < num_buttons; ++i) 
		{
			const auto button = static_cast<ButtonType>(i);

			if(buttonManager.IsPressed(button))
			{
				colors[i]		= GREEN;
				btns_millis[i]	= now;
			}else if(!buttonManager.IsDown(button))
			{
				colors[i] = DARKGREY;
			}else if(now - btns_millis[i] >= 150)	///< millisの循環にも対応
			{
				colors[i] = ORANGE;
			}
		}
	}

    ////
    /// デバッグ表示
    //
    {
        auto p_lcd = &M5.Lcd;
        {
            
            constexpr int rectW = 9;
            constexpr int margin= 1;

            int x = p_lcd->width() - (rectW+margin) * num_buttons;
            int y = 16;
            for (int i = 0; i < num_buttons; ++i) 
            {
                int effect = 0;
                if(colors[i] != GREEN)
                {
                    effect = 2;
                }
                p_lcd->fillRect(x, y, rectW, rectW, BLACK);
                p_lcd->fillRect(x + effect, y + effect, rectW - (effect<<1), rectW - (effect<<1), colors[i]);
                x += rectW + margin;
            }
        }
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

    buttonManager.Setup();
}

/**
 * @brief このloopが延々繰り返し呼ばれる。
 * 
 */
void loop()
{
    unsigned long start = millis();
    /// FPSごとの処理
    {
        displayInfo();
    }
    
    /// 通常処理
    do 
    {
        serialProc();
        buttonManager.Update();
        drawButtonsStatus();
    } while (millis() - start < (1000/fps));
}
