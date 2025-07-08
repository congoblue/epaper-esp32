## Dashboard-like display on Waveshare 3.52inch e-Paper display using ESP32

This project uses a Waveshare 3.52" e-Paper display to provide a dashboard screen.
I'm using it to show my central heating parameters but with a bit of adaption it
could be used to show anything that's on the internet.

I'm using an ESP32-S2 Saola board as I happened to have one lying around but with
a bit of modification to the IO pins it should work with any ESP32 board.

E-Paper drive code is taken from Wavehare's sample code (see link below).

IO pins are defined in global.h

You need to provide a file "credentials.h" which gives your wifi login details.

    #define SSID "Your_SSID" 
    #define PWD "Your_wifi_pwd" 


This project uses PlatformIO. To change the board if you aren't using an ESP32-Saola board
just edit the "Board" line in platformio.ini.

To adapt for different displays, see Wavehare's demo code at

https://github.com/waveshareteam/e-Paper/tree/master/Arduino

Use the folder for the specific display you are using, e.g. for this 3.52 inch display
the folder is epd3in52. Each folder contains a cpp file with setup and operating code
for the particular display.
