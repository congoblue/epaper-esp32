/**
 * ----------------------------------------------------------------------------
 * E paper test
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <esp_sleep.h>
#include <SPI.h>
#include "EPD_3in52.h"
#include "imagedata.h"
#include "epdpaint.h"

#include "global.h"

#define COLORED     0
#define UNCOLORED   1

// ----------------------------------------------------------------------------
// Global vars
// ----------------------------------------------------------------------------

uint32_t PwrOnTimer=0;
uint16_t KeyState=0;
uint8_t LastKey=0;
uint8_t KeyHit=0;
uint8_t KeyDn=0;

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    Serial.println("-BOOT-");
    
    Epd epd;
    if (epd.Init() != 0) {
        Serial.print("e-Paper init failed");
        return;
    }
    Serial.print("3.52inch e-paper demo\r\n ");
    Serial.print("e-Paper Clear\r\n ");

    epd.display_NUM(EPD_3IN52_WHITE);
    epd.lut_GC();
    epd.refresh();

    epd.SendCommand(0x50);
    epd.SendData(0x17);

    delay(2000);

    UBYTE image[700];
    Paint paint(image, 200, 25);    // width should be the multiple of 8   

    paint.SetRotate(ROTATE_0);
    paint.Clear(COLORED);

     Serial.print("Drawing:BlackImage\r\n ");
     paint.DrawStringAt(0, 0, "e-Paper Demo", &Font24, UNCOLORED);
     epd.display_part(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
     epd.lut_GC();
     epd.refresh();
     delay(2000);
    
    
    /*epd.display(IMAGE_DATA);
    epd.lut_GC();
    epd.refresh();
    delay(2000);*/
    
    Serial.print("clear and sleep......\r\n ");
    epd.Clear();
    delay(2000);
    epd.sleep();
    Serial.print("end\r\n ");

}



// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() 
{
    Serial.print(".");
    delay(250);
}