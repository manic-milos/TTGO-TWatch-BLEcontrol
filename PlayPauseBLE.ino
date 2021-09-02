#include <BleKeyboard.h>

BleKeyboard bleKeyboard;


#include "config.h"

TTGOClass *ttgo;

char battery[20];
char buf[128];
void setup()
{
    Serial.begin(115200);
    Serial.println(getCpuFrequencyMhz());
    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    lv_disp_trig_activity(NULL);
    ttgo->openBL();

    //! begin motor attach to 33 pin , In TWatch-2020 it is IO4
    ttgo->motor_begin();

    int battPercentage=ttgo->power->getBattPercentage();
    sprintf(battery, "%d\%", battPercentage);
    ttgo->tft->fillScreen(TFT_BLACK);
    ttgo->tft->setTextColor(TFT_WHITE, TFT_BLACK);
    ttgo->tft->drawString(battery,  50, 100, 2);
    bleKeyboard.setBatteryLevel(battPercentage);

    //attach touch screen interrupt pin
    pinMode(TP_INT, INPUT);

    
    //ble keyboard
    Serial.println("Starting BLE work!");
    bleKeyboard.begin();
}

bool screenOn=true;

void sleep()
{
  setCpuFrequencyMhz(50);
//  esp_light_sleep_start();
  ttgo->closeBL();        // switch off backlight
//  ttgo->stopLvglTick();   // Pause LVGL handler
  ttgo->displaySleep();   // switch off screen 
}

void awake()
{    
  setCpuFrequencyMhz(240);
//  ttgo->startLvglTick();
  ttgo->openBL();             // Backlight
  ttgo->displayWakeup();      // LVGL handler
  lv_disp_trig_activity(NULL); 
}

void loop()
{
  
    ttgo->power->readIRQ();
    if (ttgo->power->isPEKShortPressIRQ())
    {
      Serial.println("short press");
      if(ttgo->bl->isOn())
        sleep();
      else
      {
        awake();
      }
      ttgo->power->clearIRQ();
      int battPercentage=ttgo->power->getBattPercentage();
      sprintf(battery, "%d\%", battPercentage);
      ttgo->tft->fillScreen(TFT_BLACK);
      ttgo->tft->setTextColor(TFT_WHITE, TFT_BLACK);
      ttgo->tft->drawString(battery,  50, 100, 2);
      bleKeyboard.setBatteryLevel(battPercentage);
      screenOn=!screenOn;
      delay(500);
    }
    ttgo->power->clearIRQ();
    if(screenOn)
    {
      int16_t x,y;
      if (ttgo->getTouch(x,y) )
      {
          ttgo->motor->onec();
          if(x>180)
          {
            if(y>180)
            {
              Serial.println("Sending volume up media key...");
              bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
            }
            else if(y<80)
            {
              Serial.println("Sending volume down media key...");
              bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
            }
          }
          else if(x<80)
          {
            if(y>180)
            {
              Serial.println("Sending next track media key...");
              bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
            }
            else if(y<80)
            {
              Serial.println("Sending previous track media key...");
              bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
            }
          }
          else
          {
            Serial.println("Sending Play/Pause media key...");
            bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
          }
          delay(100);
      }
    }
}
