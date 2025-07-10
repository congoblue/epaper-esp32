/**
 * ----------------------------------------------------------------------------
 * E paper test
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <esp_sleep.h>
#include <SPI.h>
#include "EPD_3in52.h"
#include "epdpaint.h"
#include "graphics.h"
#include "credentials.h"

#include "global.h"
#include "main.h"

#define COLORED     0
#define UNCOLORED   1

// ----------------------------------------------------------------------------
// Global vars
// ----------------------------------------------------------------------------

uint32_t PwrOnTimer=0;

Epd epd;


// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    Serial.println("-BOOT-");
    
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

    SetRotate(ROTATE_90);
    Clear(COLORED);

    Serial.print("Drawing:BlackImage\r\n ");
    setfont(14,5);
    align=ALIGN_CENTRE;
    colour=1;
    backcolour=0;
    setxy(WIDTH/2,180);
    putstr_align("Hello World");
    setxy(WIDTH/2,220);
    putstr_align("This is Waveshare e-paper display");
    epd.display_part(GetImage(), 0, 0, HEIGHT, WIDTH); //width,height if not rotated
    epd.lut_GC();
    epd.refresh();

    delay(5000);
    
  
    
    Serial.print("clear and sleep......\r\n ");
    //epd.Clear();
    //delay(2000);
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