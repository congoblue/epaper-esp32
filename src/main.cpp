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
#include <ArduinoJson.h>
#include "EPD_3in52.h"
#include "epdpaint.h"
#include "graphics.h"
#include "credentials.h"

#include "global.h"
#include "main.h"

#define COLORED     0
#define UNCOLORED   1

#define NEWS_URL        "https://feeds.bbci.co.uk/news/rss.xml"
#define GEO_URL         "http://ip-api.com/json/?fields=status,city,lat,lon"
#define WEATHER_URL     "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f" \
                        "&current=temperature_2m,wind_speed_10m,pressure_msl" \
                        "&hourly=temperature_2m,precipitation_probability,weather_code&forecast_hours=24&wind_speed_unit=mph&timezone=auto"
#define NUM_HEADLINES   3
#define TITLE_LEN       160
#define WIFI_TIMEOUT_MS 20000
#define REFRESH_US      (30ULL*60*1000000)  //1 hour
#define RETRY_US        (5ULL*60*1000000)   //5 mins after a failure

// screen layout (rotated, WIDTH x HEIGHT)
#define SPLIT_X         240                 //divider between headlines (left 2/3) and weather (right 1/3)
#define HEADER_H        25
#define WEATHER_CX      ((SPLIT_X+WIDTH)/2)

// LATITUDE, LONGITUDE and LOCATION_NAME can be defined in credentials.h,
// otherwise location is looked up from the public IP address

#ifndef TIMEZONE
#define TIMEZONE        "GMT0BST,M3.5.0/1,M10.5.0"  //UK, POSIX TZ format
#endif
#define NTP_SERVER      "pool.ntp.org"

// ----------------------------------------------------------------------------
// Global vars
// ----------------------------------------------------------------------------

Epd epd;

char headlines[NUM_HEADLINES][TITLE_LEN];
char fetchtime[16];     //empty if time sync failed

struct {
    bool ok;
    float temp;         //C
    float wind;         //mph
    float pressure;     //hPa, sea level
    float tmin, tmax;   //C, next 24 hours
    int rain;           //% chance in next hour
    const char *raintype;
    char place[32];
} weather;


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
// Local time from NTP, eg "Sat 12:45"
// ----------------------------------------------------------------------------

void GetFetchTime(void)
{
    struct tm t;

    fetchtime[0]=0;
    if (getLocalTime(&t, 5000)) strftime(fetchtime, sizeof(fetchtime), "%a %H:%M", &t);
    else Serial.println("NTP time sync failed");
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
// GET a url and parse the JSON response
// ----------------------------------------------------------------------------

bool GetJson(const char *url, JsonDocument &doc)
{
    WiFiClientSecure sclient;
    WiFiClient client;
    HTTPClient http;
    bool secure=(strncmp(url, "https", 5)==0);
    int code;

    sclient.setInsecure(); //no cert check, public read only data
    http.useHTTP10(true);
    http.setTimeout(10000);
    if (!http.begin(secure ? (WiFiClient &)sclient : client, url)) return false;

    code=http.GET();
    if (code!=HTTP_CODE_OK)
    {
        Serial.printf("HTTP GET %s failed: %d\r\n", url, code);
        http.end();
        return false;
    }
    DeserializationError err=deserializeJson(doc, http.getStream());
    http.end();
    if (err) Serial.printf("JSON error: %s\r\n", err.c_str());
    return !err;
}

// ----------------------------------------------------------------------------
// Describe precipitation from a WMO weather code
// ----------------------------------------------------------------------------

const char *RainType(int code)
{
    switch (code)
    {
        case 51: return "Light drizzle";
        case 53: return "Drizzle";
        case 55: return "Heavy drizzle";
        case 56: case 57: return "Freezing drizzle";
        case 61: return "Light rain";
        case 63: return "Rain";
        case 65: return "Heavy rain";
        case 66: case 67: return "Freezing rain";
        case 71: return "Light snow";
        case 73: return "Snow";
        case 75: return "Heavy snow";
        case 77: return "Snow grains";
        case 80: return "Light showers";
        case 81: return "Showers";
        case 82: return "Heavy showers";
        case 85: return "Snow showers";
        case 86: return "Heavy snow shwrs";
        case 95: return "Thunderstorm";
        case 96: case 99: return "Thunder + hail";
        default: return "No rain";  //0-48 are clear, cloud and fog
    }
}

// ----------------------------------------------------------------------------
// Fetch current weather from open-meteo.com
// ----------------------------------------------------------------------------

bool FetchWeather(void)
{
    JsonDocument doc;
    float lat, lon;
    char url[300];

    weather.ok=false;
#if defined(LATITUDE) && defined(LONGITUDE)
    lat=LATITUDE;
    lon=LONGITUDE;
  #ifdef LOCATION_NAME
    strlcpy(weather.place, LOCATION_NAME, sizeof(weather.place));
  #else
    strlcpy(weather.place, "Weather", sizeof(weather.place));
  #endif
#else
    if (!GetJson(GEO_URL, doc) || strcmp(doc["status"] | "", "success")!=0) return false;
    lat=doc["lat"];
    lon=doc["lon"];
    strlcpy(weather.place, doc["city"] | "Weather", sizeof(weather.place));
    doc.clear();
#endif

    snprintf(url, sizeof(url), WEATHER_URL, lat, lon);
    if (!GetJson(url, doc)) return false;

    JsonObject cur=doc["current"];
    JsonArray rain=doc["hourly"]["precipitation_probability"];
    JsonArray code=doc["hourly"]["weather_code"];
    JsonArray temps=doc["hourly"]["temperature_2m"];
    if (cur.isNull() || rain.isNull() || code.isNull() || temps.size()==0) return false;
    weather.tmin=weather.tmax=temps[0];
    for (float t : temps)
    {
        weather.tmin=min(weather.tmin, t);
        weather.tmax=max(weather.tmax, t);
    }
    weather.temp=cur["temperature_2m"];
    weather.wind=cur["wind_speed_10m"];
    weather.pressure=cur["pressure_msl"];
    weather.rain=max(rain[0] | 0, rain[1] | 0); //next 60 mins spans this hour and the next
    weather.raintype=RainType(max(code[0] | 0, code[1] | 0)); //higher code is the worse weather
    weather.ok=true;
    Serial.printf("%s: %.1fC (%.0f-%.0f), %.0fmph, %.0fhPa, %s %d%%\r\n", weather.place, weather.temp, weather.tmin, weather.tmax, weather.wind, weather.pressure, weather.raintype, weather.rain);
    return true;
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

void PutStrCentred(const char *s, int cx, int y)
{
    setxy(cx-linewidth((char *)s)/2, y);
    putstr((char *)s);
}

void PutStrRight(const char *s, int rx, int y)
{
    setxy(rx-linewidth((char *)s), y);
    putstr((char *)s);
}

void DrawHeadlines(int count)
{
    char buf[256];
    char *p;
    int i, y, lines, lineh;

    setfont(14,5);
    lineh=14+linespacing;
    setxy(0,4);
    putstr((char *)"BBC News");
    PutStrRight(fetchtime, SPLIT_X-4, 4);

    if (count==0)
    {
        PutStrCentred("Unavailable", SPLIT_X/2, HEIGHT/2-7);
        return;
    }

    //wrap within the left panel, restore full width afterwards
    _width=SPLIT_X-4;
    wordwrap=0;
    align=ALIGN_LEFT;
    y=HEADER_H+8;
    for (i=0; i<count && y+lineh<=HEIGHT; i++)
    {
        wrap(headlines[i], buf);
        //cut off any lines that would run off the bottom of the screen
        lines=(HEIGHT-y)/lineh;
        for (p=buf; *p!=0; p++)
        {
            if (*p==13 && --lines==0) {*p=0; break;}
        }
        setxy(0,y);
        putstr(buf);
        y=yp+lineh+8; //yp is top of last line drawn
        if (i<count-1) DrawHorizontalLine(0, y-5, SPLIT_X, COLORED);
    }
    _width=WIDTH;
    wordwrap=1;
}

void FormatTemp(char *buf, int len, float t)
{
    snprintf(buf, len, "%.0f", t);
    if (strcmp(buf, "-0")==0) strlcpy(buf, "0", len);
}

#define DEG_W 7     //width of small degree symbol

//number then degree symbol and C at the current position, normal size
void PutTempSmall(char *num, int y)
{
    putstr(num);
    DrawCircle(xp+3, y+3, 2, COLORED);
    setxy(xp+DEG_W, y);
    putstr((char *)"C");
}

void DrawReading(const char *label, const char *value, int y)
{
    setfont(14,6);
    PutStrCentred(label, WEATHER_CX, y);
    setfont(14,5);
    PutStrCentred(value, WEATHER_CX, y+15);
}

void DrawWeather(void)
{
    char buf[16];
    int i, x, y, w;

    setfont(14,5);
    PutStrCentred(weather.place, WEATHER_CX, 4);

    if (!weather.ok)
    {
        PutStrCentred("Unavailable", WEATHER_CX, HEIGHT/2-7);
        return;
    }

    //big temperature, 3x magnified, with a drawn degree symbol
    FormatTemp(buf, sizeof(buf), weather.temp);
    w=3*linewidth(buf)+12+3*linewidth((char *)"C");
    x=WEATHER_CX-w/2;
    y=HEADER_H+5;
    setxy(x,y);
    for (i=0; buf[i]!=0; i++) disp_putc_big(buf[i], 3);
    DrawCircle(xp+5, y+5, 4, COLORED);
    DrawCircle(xp+5, y+5, 3, COLORED);
    setxy(xp+12, y);
    disp_putc_big('C', 3);

    //24h min - max below, each with a small drawn degree symbol
    char buf2[16];
    FormatTemp(buf, sizeof(buf), weather.tmin);
    FormatTemp(buf2, sizeof(buf2), weather.tmax);
    w=linewidth(buf)+linewidth((char *)" - ")+linewidth(buf2)+2*(DEG_W+linewidth((char *)"C"));
    y+=45;
    setxy(WEATHER_CX-w/2, y);
    PutTempSmall(buf, y);
    putstr((char *)" - ");
    PutTempSmall(buf2, y);

    //narrow panel, so label above value
    y=HEADER_H+78;
    snprintf(buf, sizeof(buf), "%.0f mph", weather.wind);
    DrawReading("Wind", buf, y);

    y+=44;
    snprintf(buf, sizeof(buf), "%.0f hPa", weather.pressure);
    DrawReading("Pressure", buf, y);

    y+=44;
    DrawReading("Rain 1hr", weather.raintype, y);
    snprintf(buf, sizeof(buf), "%d%%", weather.rain);
    PutStrCentred(buf, WEATHER_CX, y+30);
}

void DrawScreen(int count)
{
    Clear(UNCOLORED);
    colour=COLORED;
    backcolour=UNCOLORED;
    autofontsize=0;
    align=ALIGN_LEFT;
    valign=VALIGN_TOP;

    DrawHeadlines(count);
    DrawWeather();

    DrawHorizontalLine(0, HEADER_H-2, WIDTH, COLORED);
    DrawHorizontalLine(0, HEADER_H-1, WIDTH, COLORED);
    DrawVerticalLine(SPLIT_X, 0, HEIGHT, COLORED);
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

    if (ConnectWiFi())
    {
        configTzTime(TIMEZONE, NTP_SERVER); //syncs in the background while we fetch
        count=FetchHeadlines();
        FetchWeather();
        GetFetchTime();
    }

    if (count>0 || weather.ok)
    {
        DrawScreen(count);
        ShowScreen();
        Sleep((count>0 && weather.ok) ? REFRESH_US : RETRY_US);
    }

    DrawMessage(WiFi.status()==WL_CONNECTED ? "Could not load headlines or weather" : "WiFi connection failed");
    ShowScreen();
    Sleep(RETRY_US);
}



// ----------------------------------------------------------------------------
// Main control loop (not reached, device deep sleeps between refreshes)
// ----------------------------------------------------------------------------

void loop()
{
}
