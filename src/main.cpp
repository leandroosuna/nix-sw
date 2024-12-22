#include "main.h"

// #define CTS328_SLAVE_ADDRESS (0x1A)
TFT_eSPI tft = TFT_eSPI();
// TouchLib touch(Wire, PIN_IIC_SDA, PIN_IIC_SCL, CTS328_SLAVE_ADDRESS, PIN_TOUCH_RES);

void setup()
{
    
    gpio_hold_dis((gpio_num_t)PIN_TOUCH_RES);

    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    pinMode(PIN_TOUCH_RES, OUTPUT);
    digitalWrite(PIN_TOUCH_RES, LOW);
    delay(250);
    digitalWrite(PIN_TOUCH_RES, HIGH);

    pinMode(PIN_BAT_VOLT, INPUT);

    Serial.begin(115200);
    //while(!Serial);
    debugln("ESP32 Started");
    
    //pinMode(12, OUTPUT);
    tft.begin();
    for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++) {
        tft.writecommand(lcd_st7789v[i].cmd);
        for (int j = 0; j < lcd_st7789v[i].len & 0x7f; j++) {
            tft.writedata(lcd_st7789v[i].data[j]);
        }

        if (lcd_st7789v[i].len & 0x80) {
            delay(120);
        }
    }
    //tft.setRotation(1); //horizontal
    tft.setRotation(0); //vertical
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL);
    TouchHandler::init();

    debugln("TFT CFG ok");
    // tft.pushImage(0, 0, 320, 170, (uint16_t *)img_logo);

    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 128);
    Serial.println("Blacklight control ok");

    tft.fillScreen(TFT_BLACK);
    
    tft.drawString("NIX SW", 0, 0);
    
    delay(500);
}
int c = 0;
int t = 0;
int speed=0;



void loop()
{
    static unsigned long lastTime = 0;
    unsigned long currentTime = millis();
    char str_buf[100];

    if (currentTime - lastTime >= 10)
    {
        TouchHandler::update();
    // }

    // if (currentTime - lastTime >= 50)
    // {
        lastTime = currentTime;

        Gesture gesture = TouchHandler::getGesture();

        if (gesture.type != GestureType::NONE)
        {
            tft.fillScreen(TFT_BLACK);
            sprintf(str_buf, "%c %d,%d\n%d,%d", getGestureChar(gesture.type), gesture.start.x, gesture.start.y, gesture.end.x, gesture.end.y);
            tft.drawString(str_buf, 0, 0);

            debugln(str_buf);
        }
    }

    delay(1); // Small delay to prevent watchdog timeout
}
// esp_adc_cal_characteristics_t adc_chars;
    // if(t % 100 == 0)
    // {
    //     // Get the internal calibration value of the chip
    //     esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    //     uint32_t raw = analogRead(PIN_BAT_VOLT);
    //     uint32_t v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2; //The partial pressure is one-half

    //     tft.fillScreen(TFT_BLACK);
    //     tft.setCursor(0, 0);

    //     // If the battery is not connected, the ADC detects the charging voltage of TP4056, which is inaccurate.
    //     // Can judge whether it is greater than 4300mV. If it is less than this value, it means the battery exists.
    //     if (v1 > 4300) {
    //         tft.print("No battery connected!");
    //     } else {
    //         float bat = (float)v1 * 0.001;
    //         tft.print(bat);
    //         tft.print("V");
    //     }
    // }


// }

char getGestureChar(GestureType type)
{
    switch(type)
    {
        case GestureType::TAP : return 't';
        case GestureType::SWIPE : return 's';
        case GestureType::EXPAND : return 'e';
        case GestureType::PINCH : return 'p';
        case GestureType::DRAG : return 'd';
        case GestureType::DROP : return 'D';
        
    }
    return 0;
}