#include <CRC32.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include "Effect/Effect.h"
#include <WiFiUdp.h>
#include <arduino-timer.h>

auto timer = timer_create_default();
WiFiUDP UDP;
IPAddress IPAddr = IPAddress(10, 0, 0, 1);
const int UDP_PORT = 2000;

void setupWifi() {
    static const char* ssid = "DragonBand";
    static const char* password = "GoRockBand!";
    WiFi.softAP(ssid, password, 1, 0, 100);
    IPAddress IPNM = IPAddress(255, 255, 255, 0);
    WiFi.softAPConfig(IPAddr, IPAddr, IPNM);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("\nAP IP address: ");
    Serial.println(IP);
}

void setupUDP() {
    // Begin listening to UDP port
    UDP.begin(UDP_PORT);
    Serial.print("Listening on UDP port ");
    Serial.println(UDP_PORT);
}

void setupSerial() {
    Serial.begin(115200);
    Serial.print("\n\nProps Server\nBuilt on: ");
    Serial.println(__DATE__ " @ " __TIME__ "\n\n");
}

void hw_wdt_disable() {
    *((volatile uint32_t*)0x60000900) &= ~(1);  // Hardware WDT OFF
}

void hw_wdt_enable() {
    *((volatile uint32_t*)0x60000900) |= 1;  // Hardware WDT ON
}

static RifleFX riflefx;
static PoleFX polefx;

int send_mcast(uint8_t *pData, size_t len) {
    int ret;
    static const IPAddress mcastaddr = IPAddress(10, 0, 0, 255);
    static const IPAddress unicast = IPAddress(10, 0, 0, 100);

    ret = UDP.beginPacketMulticast(mcastaddr, UDP_PORT, WiFi.softAPIP(), 2);
    UDP.write(pData, len);
    ret = UDP.endPacket();
    Serial.print("UDP.endPacket ");
    return ret;
}

bool send_config(void*) {
    int ret;
    ret = send_mcast((uint8_t*)(&polefx.effect[0]), sizeof(polefx.effect[0]));
    Serial.print("MCast Settings: ");
    Serial.println(ret);
    return true;
}

bool change_config(void*) {
    static int config = 0;

    switch (config) {
        case 0:
            AllRed(polefx.effect[0]);
            config = 1;
            break;
        case 1:
            AllGreen(polefx.effect[0]);
            config = 2;
            break;
        case 2:
            AllBlue(polefx.effect[0]);
            config = 3;
            break;
        case 3:
            DB23GuardYellowPurple(polefx.effect[0]);
            config = 0;
            break;


    }
    return true;

}

void setup() {
    int ret = 0;

    setupSerial();
    setupWifi();
    setupUDP();
    // timer.every(5000, rotate_mode);
    // riflefx.effect[0];
    // DB23GuardYellowPurple(polefx.effect[0]);
    // @FIXME @TODO - hardcoded length
    polefx.effect[0].size = 250;

    // DB23GuardYellowPurple(riflefx.effect[0]);

    // uint32_t crc =
    //     CRC32::calculate(&riflefx.effect[0], sizeof(riflefx.effect[0]));

    delay(250);

    timer.every(1000, send_config);
    timer.every(5000, change_config);



}

void loop() { 
    timer.tick();
}


#if 0
bool rotate_mode(void*) {
    static const char* modes[] = {"aniylw", "white", "red",
                                  "green",  "blue",  "wob"};
    static int i = 0;
    // static const IPAddress mcast = IPAddress(255,255,255,255);
    static const IPAddress mcast = IPAddress(10, 0, 0, 255);
    static const IPAddress unicast = IPAddress(10, 0, 0, 100);
    Serial.println(modes[i]);
    int ret = 0;

    Serial.print("mode ");
    Serial.println(i);

    ret = UDP.beginPacketMulticast(mcast, UDP_PORT, WiFi.softAPIP(), 2);
    // ret = UDP.beginPacket(unicast, UDP_PORT);
    Serial.print("UDP.beginPacket ");
    Serial.println(ret);

    UDP.write(modes[i]);
    i = (i + 1) % (sizeof(modes) / sizeof(char*));

    ret = UDP.endPacket();
    Serial.print("UDP.endPacket ");
    Serial.println(ret);
    // keep timer running
    return true;
}
#endif
