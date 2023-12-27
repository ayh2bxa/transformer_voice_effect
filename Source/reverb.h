#define CHANNELS 2
#define NUM_COMBS 8
#define NUM_APS 4

struct CombFilter {
    float *buffer;
    unsigned short buflen;
    int ind;
    float dampFactor;
    float decayFactor;
    float prev;
};

struct AllPassFilter {
    float *buffer;
    unsigned short buflen;
    int ind;
    float decayFactor;
};

struct SchroederReverb {
    struct CombFilter *Combs[CHANNELS][NUM_COMBS];
    struct AllPassFilter *APs[CHANNELS][NUM_APS];
    float wet;
    float wet1;
    float wet2;
    float width;
    float size;
};

float applyComb(float input, struct CombFilter *Comb);

float applyAP(float input, struct AllPassFilter *AP);

void applyReverb(struct SchroederReverb *R, float *left, float *right, int numSamples);

void freeReverb(struct SchroederReverb *R);

struct SchroederReverb *initReverb(float width, float wet, float dampFactor, float size);
