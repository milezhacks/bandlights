#pragma once

#include <stddef.h>
#include <stdint.h>
#include <Adafruit_NeoPixel.h>

#define PROP_UNKNOWN ((uint32_t)-1)
#define PROP_RIFLE ((uint32_t)0xBA9D0000)
#define PROP_STAFF ((uint32_t)0xBA9D0001)

#define NLEDS_POLE ((uint32_t)250)
#define NLEDS_RIFLE ((uint32_t)50)
#define NLEDS_MAX (NLEDS_POLE)

#define EFFECTS_MAX (8)

template <size_t NLED>
struct EffectData {
    uint32_t type;
    uint32_t size;
    uint32_t csum;
    // Allocating storage for worst-case prop isn't efficient
    uint32_t colors[NLED];
};

template <size_t NLED>
struct Effects {
    EffectData<NLED> effect[EFFECTS_MAX];
};

template <size_t NLED>
void DB23GuardYellowPurple(EffectData<NLED> &e) {
    static int phi = 0;
    static bool bInited = false;
    static const int freq = 8;
#if 0
    // For each pixel in strip...
    // we have to skip ahead two segments because we do yellow then purple
    for (int j = 0; j < NLED; j += 2 * NLED / freq) {
        // for each pixel in this segment (strip/freq)
        for (int i = 0; i < NLED / freq; i++) {
            unsigned int p1 =
                Adafruit_NeoPixel::sine8(((uint32_t)255 * i * freq) / (NLED));
            // BBRRGGgg ?
            int purple = Adafruit_NeoPixel::Color(p1, 0, p1, 0);
            int yellow = Adafruit_NeoPixel::Color(p1, p1, 0, 0);
            e.colors[j + i] = purple;
            e.colors[j + i + (NLED / freq)] = yellow;
        }
    }
#else
    int j = 0;
    int purple = Adafruit_NeoPixel::Color(255, 0, 255, 0);
    int yellow = Adafruit_NeoPixel::Color(255, 255, 0, 0);
    while(j < NLED) {
        for(int i = 0; i < 25 && j < NLED; i++, j++) {
            e.colors[j] = purple;
        }
        for(int i = 0; i < 25 && j < NLED; i++, j++) {
            e.colors[j] = yellow;
        }
    }
#endif
}

template <size_t NLED>
void AllRed(EffectData<NLED> &e) {
    for (int j = 0; j < NLED; j ++) {
        e.colors[j] = Adafruit_NeoPixel::Color(255, 0, 0, 0);
    }
}

template <size_t NLED>
void AllGreen(EffectData<NLED> &e) {
    for (int j = 0; j < NLED; j ++) {
        e.colors[j] = Adafruit_NeoPixel::Color(0, 255, 0, 0);
    }
}

template <size_t NLED>
void AllBlue(EffectData<NLED> &e) {
    for (int j = 0; j < NLED; j ++) {
        e.colors[j] = Adafruit_NeoPixel::Color(0, 0, 255, 0);
    }
}

typedef Effects<NLEDS_RIFLE> RifleFX;
typedef Effects<NLEDS_POLE> PoleFX;


#if 0
class Effect {
  public:
};
#endif