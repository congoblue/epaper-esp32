/**
 * ----------------------------------------------------------------------------
 * E paper BBC headlines
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <esp_sleep.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "EPD_3in52.h"
#include "epdpaint.h"
#include "graphics.h"
#include "credentials.h"

#include "global.h"
#include "main.h"

#define COLORED     0
#define UNCOLORED   1

#define NEWS_URL        "https://feeds.bbci.co.uk/news/rss.xml"
#define NUM_HEADLINES   3
#define TITLE_LEN       160
#define WIFI_TIMEOUT_MS 20000
#define REFRESH_US      (60ULL*60*1000000)  //1 hour
#define RETRY_US        (5ULL*60*1000000)   //5 mins after a failure

// ----------------------------------------------------------------------------
// Global vars
// ----------------------------------------------------------------------------

Epd epd;

char headlines[NUM_HEADLINES][TITLE_LEN];


// ----------------------------------------------------------------------------
// WiFi
// ----------------------------------------------------------------------------

bool ConnectWiFi(void)
{
    uint32_t start=millis();

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PWD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status()!=WL_CONNECTED)
    {
        if (millis()-start>WIFI_TIMEOUT_MS)
        {
            Serial.println(" timeout");
            return false;
        }
        Serial.print(".");
        delay(250);
    }
    Serial.print(" connected, IP ");
    Serial.println(WiFi.localIP());
    return true;
}

// ----------------------------------------------------------------------------
// Convert an RSS title to plain ASCII the fonts can draw (chars 32-126 only)
// ----------------------------------------------------------------------------

void CleanTitle(String t, char *out, int len)
{
    unsigned int i;
    int n=0;
    unsigned char ch;

    t.replace("<![CDATA[", "");
    t.replace("]]>", "");
    t.replace("&quot;", "\"");
    t.replace("&apos;", "'");
    t.replace("&#39;", "'");
    t.replace("&lt;", "<");
    t.replace("&gt;", ">");
    t.replace("&amp;", "&"); //last so we don't double decode
    t.trim();

    for (i=0; i<t.length() && n<len-4; i++)
    {
        ch=t[i];
        if (ch>=32 && ch<127) out[n++]=ch;
        else if (ch==0xE2 && i+2<t.length() && (unsigned char)t[i+1]==0x80) //UTF-8 punctuation
        {
            ch=t[i+2];
            if (ch==0x98 || ch==0x99) out[n++]='\'';
            else if (ch==0x9C || ch==0x9D) out[n++]='"';
            else if (ch==0x93 || ch==0x94) out[n++]='-';
            else if (ch==0xA6) {out[n++]='.'; out[n++]='.'; out[n++]='.';}
            i+=2;
        }
        else if (ch==0xC2 && i+1<t.length() && (unsigned char)t[i+1]==0xA3) //pound sign
        {
            out[n++]='G'; out[n++]='B'; out[n++]='P';
            i+=1;
        }
        else if (ch>=0xC0) i+=(ch>=0xF0)?3:(ch>=0xE0)?2:1; //skip any other multibyte char
    }
    out[n]=0;
}

// ----------------------------------------------------------------------------
// Fetch the BBC news RSS feed and pull out the first item titles
// returns number of headlines found
// ----------------------------------------------------------------------------

int FetchHeadlines(void)
{
    WiFiClientSecure client;
    HTTPClient http;
    int count=0, code;
    char ch;
    String t;

    client.setInsecure(); //no cert check, we only read public headlines
    http.useHTTP10(true); //avoid chunked encoding so the stream is plain XML
    http.setTimeout(10000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (!http.begin(client, NEWS_URL)) return 0;

    code=http.GET();
    if (code!=HTTP_CODE_OK)
    {
        Serial.printf("HTTP GET failed: %d\r\n", code);
        http.end();
        return 0;
    }

    Stream &s=http.getStream();
    t.reserve(TITLE_LEN*2);
    while (count<NUM_HEADLINES)
    {
        //skip the channel title, only want <item> titles
        if (!s.find("<item>") || !s.find("<title>")) break;
        t="";
        while (s.readBytes(&ch, 1)==1)
        {
            t+=ch;
            if (t.endsWith("</title>") || t.length()>1000) break;
        }
        if (!t.endsWith("</title>")) break;
        t.remove(t.length()-8);
        CleanTitle(t, headlines[count], TITLE_LEN);
        Serial.printf("%d: %s\r\n", count+1, headlines[count]);
        count++;
    }
    http.end();
    return count;
}

// ----------------------------------------------------------------------------
// Draw to the frame buffer and update the display
// ----------------------------------------------------------------------------

void ShowScreen(void)
{
    //lut_GC alternates waveforms each call, so it must be a white clear followed by the image
    epd.display_NUM(EPD_3IN52_WHITE);
    epd.lut_GC();
    epd.refresh();

    epd.SendCommand(0x50);
    epd.SendData(0x17);

    epd.display_part(GetImage(), 0, 0, HEIGHT, WIDTH); //width,height if not rotated
    epd.lut_GC();
    epd.refresh();
}

void DrawHeadlines(int count)
{
    int i, y;

    Clear(UNCOLORED);
    setfont(14,5);
    colour=COLORED;
    backcolour=UNCOLORED;
    wordwrap=1;
    autofontsize=0;
    valign=VALIGN_TOP;

    align=ALIGN_CENTRE;
    setxy(0,4);
    putstr_align((char *)"BBC News - Top Stories");
    DrawHorizontalLine(0, 23, WIDTH, COLORED);
    DrawHorizontalLine(0, 24, WIDTH, COLORED);

    align=ALIGN_LEFT;
    y=34;
    for (i=0; i<count; i++)
    {
        setxy(0,y);
        putstr(headlines[i]);
        y=yp+14+linespacing+8; //yp is top of last line drawn
        if (i<count-1) DrawHorizontalLine(0, y-5, WIDTH, COLORED);
    }
}

void DrawMessage(const char *msg)
{
    Clear(UNCOLORED);
    setfont(14,5);
    colour=COLORED;
    backcolour=UNCOLORED;
    autofontsize=0;
    align=ALIGN_CENTRE;
    valign=VALIGN_CENTRE;
    putstr_align((char *)msg);
}

// ----------------------------------------------------------------------------
// Sleep until next refresh, setup() runs again on wake
// ----------------------------------------------------------------------------

void Sleep(uint64_t us)
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    epd.sleep();
    Serial.printf("Sleeping for %u mins\r\n", (unsigned int)(us/60000000ULL));
    Serial.flush();
    esp_sleep_enable_timer_wakeup(us);
    esp_deep_sleep_start();
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void setup() {
    int count=0;

    Serial.begin(115200);
    Serial.println("-BOOT-");

    if (epd.Init() != 0) {
        Serial.println("e-Paper init failed");
        Sleep(RETRY_US);
    }
    SetRotate(ROTATE_90);

    if (ConnectWiFi()) count=FetchHeadlines();

    if (count>0)
    {
        DrawHeadlines(count);
        ShowScreen();
        Sleep(REFRESH_US);
    }

    DrawMessage(WiFi.status()==WL_CONNECTED ? "Could not load headlines" : "WiFi connection failed");
    ShowScreen();
    Sleep(RETRY_US);
}



// ----------------------------------------------------------------------------
// Main control loop (not reached, device deep sleeps between refreshes)
// ----------------------------------------------------------------------------

void loop()
{
}
