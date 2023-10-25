#pragma once

#include <stddef.h>
#include <stdint.h>
#include <Adafruit_NeoPixel.h>

#define PROP_UNKNOWN ((uint32_t)-1)
#define PROP_RIFLE ((uint32_t)0xBA9D0000)
#define PROP_STAFF ((uint32_t)0xBA9D0001)

#define NLEDS_POLE ((uint32_t)228)
#define NLEDS_RIFLE ((uint32_t)50)
#define NLEDS_MAX (NLEDS_POLE)

#define EFFECTS_MAX (8)

template <size_t NLED>
struct EffectData {
    uint32_t type;
    uint32_t size;
    uint32_t csum;
    uint32_t frame_period_ms;
    // Allocating storage for worst-case prop isn't efficient
    uint32_t colors[NLED];
};

template <size_t NLED>
struct Effects {
    EffectData<NLED> effect[EFFECTS_MAX];
};

#if 0
template <size_t NLED>
void DB23GuardYellowPurple(EffectData<NLED>& e)
{
    static int phi = 0;
    static bool bInited = false;
    static const int freq = 8;
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
}
#endif

template <size_t NLED>
void AlternatingColor(EffectData<NLED>& e, uint32_t color1, uint32_t c1_len, uint32_t color2, uint32_t c2_len) {
    unsigned int j = 0;
    while (j < NLED) {
        for (unsigned int i = 0; i < c1_len && j < NLED; i++, j++) {
            e.colors[j] = color1;
        }
        for (unsigned int i = 0; i < c2_len && j < NLED; i++, j++) {
            e.colors[j] = color2;
        }
    }
    e.size = NLED;
}

template <size_t NLED>
void AllRed(EffectData<NLED>& e) {
    for (int j = 0; j < NLED; j++) {
        e.colors[j] = Adafruit_NeoPixel::Color(255, 0, 0, 0);
    }
}

template <size_t NLED>
void AllGreen(EffectData<NLED>& e) {
    for (int j = 0; j < NLED; j++) {
        e.colors[j] = Adafruit_NeoPixel::Color(0, 255, 0, 0);
    }
}

template <size_t NLED>
void AllBlue(EffectData<NLED>& e) {
    for (int j = 0; j < NLED; j++) {
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