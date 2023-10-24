#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <arduino-timer.h>
#include <user_interface.h>
#include <Effect.h>

auto timer = timer_create_default();

WiFiUDP UDP;

#define STATUS D2

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_DATA_IO D3

// How many NeoPixels are attached to the Arduino?
// #define LED_COUNT  34
#define LED_COUNT 250
// #define LED_COUNT  50

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 255

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
    int retries = 50;

    WiFi.mode(WIFI_STA);
    wifi_set_sleep_type(NONE_SLEEP_T);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to: ");
    Serial.print(ssid);
    // Loop continuously while WiFi is not connected
    while ((WiFi.status() != WL_CONNECTED) && retries--) {
        delay(100);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("\nIP address: ");
        Serial.println(WiFi.localIP());

        Serial.print("BCast address: ");
        Serial.println(WiFi.broadcastIP());

        return WiFi.localIP();
    } else {
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
        Serial.print("WiFi disconnected");
        setupWifi();
        setupUDP();
    }
    return true;
}

bool toggle_led(void*) {
    static int state = 1;
    digitalWrite(STATUS, state);
    state = state ^ 1;
    return true;
}

void setupEEPROM() {
    EEPROM.begin(16384);
    EEPROM[0];
}

uint32_t frame = 0;
bool update_strip(void*) {
    strip.show(); 
    frame++;
    return true;
}

void setup() {
    pinMode(STATUS, OUTPUT);  // LED pin as output.

    timer.every(5000, check_wifi);
    timer.every(1000, toggle_led);
    timer.every(33, update_strip);
    strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.setBrightness(BRIGHTNESS);  // Set BRIGHTNESS to about 1/5 (max = 255)
    strip.fill(strip.Color(0, 0, 0, 0));
    strip.setPixelColor(255, 0, 0, 0);
    strip.show();  // Turn OFF all pixels ASAP

    setupEEPROM();
    setupUDP();
    setupSerial();
    setupWifi();
}

#if 0
void loop() {
    static const int PACKET_LEN = 255;
    static uint8_t packet[PACKET_LEN];
    const char* pMode = (const char*)packet;
    static int mode = 0;
    timer.tick();

    int packetSize = UDP.parsePacket();

    if (packetSize > 0) {
        int len = UDP.read(packet, PACKET_LEN);
        // Serial.print("UDP Read len ");
        // Serial.println(len);

        if (strncmp(pMode, "aniylw", len) == 0) {
            mode = 1;
        } else if (strncmp(pMode, "white", len) == 0) {
            mode = 2;
        } else if (strncmp(pMode, "red", len) == 0) {
            mode = 3;
        } else if (strncmp(pMode, "green", len) == 0) {
            mode = 4;
        } else if (strncmp(pMode, "blue", len) == 0) {
            mode = 5;
        } else if (strncmp(pMode, "wob", len) == 0) {
            mode = 6;
        } else {
            mode = 0;
        }
        Serial.print("Mode: ");
        Serial.println(mode);
    }

    switch (mode) {
        case 1:
            // DB23GuardYellowPurple();
            break;
        case 2:
            FullPower();
            break;
        case 3:
            strip.fill(strip.Color(255, 0, 0));
            break;
        case 4:
            strip.fill(strip.Color(0, 255, 0));
            break;
        case 5:
            strip.fill(strip.Color(0, 0, 255));
            break;
        case 6:
            whiteOverRainbow(75, 5);
            break;
    }
    if (WiFi.status() != WL_CONNECTED) {
        strip.setPixelColor(0, 255, 0, 0);
    } else {
        strip.setPixelColor(0, 0, 255, 0);
    }
    strip.show();
}
#else
PoleFX fx;
EffectData<NLEDS_POLE> tmpefct;

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
    for(int i = 0; i < tmpefct.size; i++) {
        int pxidx = (i+frame) % tmpefct.size;
        strip.setPixelColor(pxidx, tmpefct.colors[i]);
    }

    if (WiFi.status() != WL_CONNECTED) {
        strip.setPixelColor(0, Adafruit_NeoPixel::Color(255,0,0,0));
    } else {
        strip.setPixelColor(0, Adafruit_NeoPixel::Color(0,255,0,0));
    }
}
#endif
void FullPower() { strip.fill(0); }

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
    for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
        strip.setPixelColor(i, color);  //  Set pixel's color (in RAM)
        strip.show();                   //  Update strip to match
        delay(wait);                    //  Pause for a moment
    }
}

void whiteOverRainbow(int whiteSpeed, int whiteLength) {
    if (whiteLength >= strip.numPixels()) whiteLength = strip.numPixels() - 1;

    static int head = whiteLength - 1;
    static int tail = 0;
    static int loops = 3;
    static int loopNum = 0;
    static uint32_t lastTime = millis();
    static uint32_t firstPixelHue = 0;

    for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
        if (((i >= tail) && (i <= head)) ||        //  If between head & tail...
            ((tail > head) && ((i >= tail) || (i <= head)))) {
            strip.setPixelColor(i, strip.Color(255, 255, 255));  // Set white
        } else {  // else set rainbow
            int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
            strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
        }
    }

    // There's no delay here, it just runs full-tilt until the timer and
    // counter combination below runs out.

    firstPixelHue += 40;  // Advance just a little along the color wheel

    if ((millis() - lastTime) > whiteSpeed) {  // Time to update head/tail?
        if (++head >= strip.numPixels()) {     // Advance head, wrap around
            head = 0;
            if (++loopNum >= loops) return;
        }
        if (++tail >= strip.numPixels()) {  // Advance tail, wrap around
            tail = 0;
        }
        lastTime = millis();  // Save time of last movement
    }
}

void pulseWhite(uint8_t wait) {
    for (int j = 0; j < 256; j++) {  // Ramp up from 0 to 255
        // Fill entire strip with white at gamma-corrected brightness level 'j':
        strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
        strip.show();
        delay(wait);
    }

    for (int j = 255; j >= 0; j--) {  // Ramp down from 255 to 0
        strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
        strip.show();
        delay(wait);
    }
}

void rainbowFade2White(int wait, int rainbowLoops, int whiteLoops) {
    int fadeVal = 0, fadeMax = 100;

    // Hue of first pixel runs 'rainbowLoops' complete loops through the color
    // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to rainbowLoops*65536, using steps of 256 so we
    // advance around the wheel at a decent clip.
    for (uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops * 65536;
         firstPixelHue += 256) {
        for (int i = 0; i < strip.numPixels();
             i++) {  // For each pixel in strip...

            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (strip.numPixels() steps):
            uint32_t pixelHue =
                firstPixelHue + (i * 65536L / strip.numPixels());

            // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the three-argument variant, though the
            // second value (saturation) is a constant 255.
            strip.setPixelColor(
                i, strip.gamma32(
                       strip.ColorHSV(pixelHue, 255, 255 * fadeVal / fadeMax)));
        }

        strip.show();
        delay(wait);

        if (firstPixelHue < 65536) {           // First loop,
            if (fadeVal < fadeMax) fadeVal++;  // fade in
        } else if (firstPixelHue >=
                   ((rainbowLoops - 1) * 65536)) {  // Last loop,
            if (fadeVal > 0) fadeVal--;             // fade out
        } else {
            fadeVal = fadeMax;  // Interim loop, make sure fade is at max
        }
    }

    for (int k = 0; k < whiteLoops; k++) {
        for (int j = 0; j < 256; j++) {  // Ramp up 0 to 255
            // Fill entire strip with white at gamma-corrected brightness level
            // 'j':
            strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
            strip.show();
        }
        delay(1000);                      // Pause 1 second
        for (int j = 255; j >= 0; j--) {  // Ramp down 255 to 0
            strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
            strip.show();
        }
    }

    delay(500);  // Pause 1/2 second
}
