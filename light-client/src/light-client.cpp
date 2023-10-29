#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <arduino-timer.h>
#include <user_interface.h>
#include <Effect/Effect.h>

auto timer = timer_create_default();

WiFiUDP UDP;

#define STATUS D2

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_DATA_IO D3

#define LED_COUNT NLEDS_POLE

// NeoPixel brightness, 0 (min) to 255 (max)
// NOTE: The FET in the voltage regulator of the light
// poles over-heats above 128!  Adjust with caution!
#define BRIGHTNESS 128

static const int UDP_PORT = 2000;

// Declare our NeoPixel strip object:
// Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel strip(LED_COUNT, LED_DATA_IO, NEO_BRG + NEO_KHZ800);

// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

IPAddress setupWifi() {
    static const char* ssid = "DragonBand";
    static const char* password = "GoRockBand!";

    Serial.print("Connecting to: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    wifi_set_sleep_type(NONE_SLEEP_T);
    WiFi.begin(ssid, password);

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("\nIP address: ");
        Serial.println(WiFi.localIP());

        Serial.print("BCast address: ");
        Serial.println(WiFi.broadcastIP());

        return WiFi.localIP();
    }
    else {
        return IPAddress(0, 0, 0, 0);
    }
}

void setupUDP() {
    int ret;

    // Begin listening to UDP port
    ret = UDP.begin(2000);
    Serial.print("begin UDP returns ");
    Serial.println(ret);

    // ret = UDP.beginMulticast(IPAddress(10,0,0,100), IPAddress(10,0,0,255),
    // 2345); Serial.print("beginMulticast returns "); Serial.println(ret);

    Serial.print("Listening on UDP port ");
    Serial.println(UDP_PORT);
}

void setupSerial() {
    Serial.begin(115200);
    Serial.print("\n\nProps Client\nBuilt on: ");
    Serial.println(__DATE__ " @ " __TIME__ "\n\n");
}

bool check_wifi(void*) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected");
        setupWifi();
        setupUDP();
    }
    timer.in(5000, check_wifi);;
    return true;
}

uint32_t status_pixel_color;
int show_status = 1;

//
// show the status LED with a 10% duty cycle
//
bool toggle_led(void*) {
    if (!show_status) {
        if (WiFi.status() != WL_CONNECTED) {
            status_pixel_color = Adafruit_NeoPixel::Color(64, 0, 64, 0);
        }
        else {
            status_pixel_color = Adafruit_NeoPixel::Color(0, 64, 0, 0);
        }
        show_status = 1;
        // if we're on, cue ourselves in 200ms so we toggle it
        timer.in(200, toggle_led);
    }
    else
    {

        show_status = 0;
        timer.in(800, toggle_led);
    }

    return true;
}

void setupEEPROM() {
    EEPROM.begin(16384);
    EEPROM[0];
}

PoleFX fx;
EffectData<NLEDS_POLE> tmpefct;

uint32_t frame = 0;
bool update_strip(void* pCurrentFx) {
    EffectData<NLEDS_POLE>* pFct = (EffectData<NLEDS_POLE>*)pCurrentFx;

    strip.show();
    frame++;
    // @TODO - pull this update interval from the fx
    // structure so the lighting effect object can
    // dictate its own animation speed.
    timer.in(pFct->frame_period_ms, update_strip, pCurrentFx);
    Serial.print(".");
    return true;
}

void setup() {
    setupSerial();
    // setupWifi();
    // setupUDP();

    // uint32_t purple = Adafruit_NeoPixel::Color(255, 0, 255, 0);
    // uint32_t yellow = Adafruit_NeoPixel::Color(255, 255, 0, 0);
    // AlternatingColor(tmpefct, purple, 10, yellow, 10);
    Rainbow(tmpefct);
    tmpefct.frame_period_ms = 1;

    // use timer.in() instead of timer.every() so each timer function
    // can decide what the next interval is.
    timer.in(1000, check_wifi);
    timer.in(1000, toggle_led);
    timer.in(10, update_strip, (void*)&tmpefct);
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    strip.fill(strip.Color(0, 0, 0, 0));
    strip.show();  // Turn OFF all pixels ASAP

    setupEEPROM();
}

void loop() {
    timer.tick();

    int packetSize = UDP.parsePacket();
    // for now, whatever we receive over UDP broadcast is what we show
    if (packetSize > 0) {
        int len = UDP.read((char*)&tmpefct, sizeof(tmpefct));
        Serial.print("UDP Read len ");
        Serial.println(len);

    }
    // fill the strip with the buffer, rotating based on the frame
    for (unsigned int i = 0; i < tmpefct.size; i++) {
        int pxidx = (i + frame) % tmpefct.size;
        strip.setPixelColor(pxidx, tmpefct.colors[i]);
    }

    // only re-paint the status pixel(s) if we're showing status
    if (show_status) {
        strip.setPixelColor(1, status_pixel_color);
    }

}
