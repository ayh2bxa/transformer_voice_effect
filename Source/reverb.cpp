#include "reverb.h"
#include <stdlib.h>
#include <stdio.h>

float applyComb(float input, struct CombFilter *Comb) {
    float output = Comb->buffer[Comb->ind];
    Comb->prev = Comb->dampFactor*Comb->prev+output*(1.0f-Comb->dampFactor);
    Comb->buffer[Comb->ind] = Comb->decayFactor*Comb->prev+input;
    Comb->ind = (Comb->ind+1)%Comb->buflen;
    return output;
}

float applyAP(float input, struct AllPassFilter *AP) {
    float bufferValue = AP->buffer[AP->ind];
    float output = bufferValue;
    AP->buffer[AP->ind] = input;
    AP->ind = (AP->ind+1)%AP->buflen;
    return output;
}

void applyReverb(struct SchroederReverb *R, float *left, float *right, int numSamples) {
    for (int i = 0; i < numSamples; ++i)
    {
        float input = (left[i] + right[i]) * 0.015f;
        float outL = 0, outR = 0;
        
        // parallel comb filters
        for (int j = 0; j < NUM_COMBS; ++j)
        {
            outL += applyComb(input, R->Combs[0][j]);
            outR += applyComb(input, R->Combs[1][j]);
        }

        // serial all-pass filters
        for (int j = 0; j < NUM_APS; ++j)
        {
            outL = applyAP(outL, R->APs[0][j]);
            outR = applyAP(outR, R->APs[1][j]);
        }
        left[i]  = R->wet*(outL * R->wet1 + outR * R->wet2) + left[i];
        right[i] = R->wet*(outR * R->wet1 + outL * R->wet2) + right[i];
    }
}

void freeReverb(struct SchroederReverb *R) {
    for (int i = 0; i < CHANNELS; i++) {
        for (int j = 0; j < NUM_COMBS; j++) {
            free(R->Combs[i][j]->buffer);
        }
        free(R->Combs);
        for (int j = 0; j < NUM_APS; j++) {
            free(R->APs[i][j]->buffer);
        }
        free(R->APs);
    }
    free(R);
}

struct SchroederReverb *initReverb(float width, float wet, float dampFactor, float size) {
    struct SchroederReverb *R = (struct SchroederReverb *)malloc(sizeof(struct SchroederReverb));
    if (R == NULL) {
        printf("Schroeder reverb allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    R->wet = wet;
    R->width = width;
    R->wet1 = 0.5f*(1.0f+R->width);
    R->wet2 = 0.5f*(1.0f-R->width);
    R->size = size;
    unsigned short CombsDelays[NUM_COMBS] = {1050, 1200, 1310, 1380, 1400, 1450, 1550, 1690};
    unsigned short APDelays[NUM_APS] = {550, 440, 360, 220};
    for (int i = 0; i < CHANNELS; i++) {
        for (int j = 0; j < NUM_COMBS; j++) {
            R->Combs[i][j] = (struct CombFilter*)malloc(sizeof(struct CombFilter));
            if (R->Combs[i][j] == NULL) {
                printf("Comb filter allocation failed!\n");
                exit(EXIT_FAILURE);
            }
            R->Combs[i][j]->dampFactor = dampFactor;
            R->Combs[i][j]->decayFactor = 0.6f+0.35f*R->size;
            if (i == 1) {
                R->Combs[i][j]->buflen = 35+CombsDelays[j];
            } else {
                R->Combs[i][j]->buflen = CombsDelays[j];
            }
            R->Combs[i][j]->ind = 0;
            R->Combs[i][j]->prev = 0.0f;
            R->Combs[i][j]->buffer = (float*)calloc(sizeof(float), R->Combs[i][j]->buflen);
            if (R->Combs[i][j]->buffer == NULL) {
                printf("Comb filter buffer allocation failed!\n");
                exit(EXIT_FAILURE);
            }
        }
        for (int j = 0; j < NUM_APS; j++) {
            R->APs[i][j] = (struct AllPassFilter*)malloc(sizeof(struct AllPassFilter));
            if (R->APs[i][j] == NULL) {
                printf("All-pass filter allocation failed!\n");
                exit(EXIT_FAILURE);
            }
            if (i == 1) {
                R->APs[i][j]->buflen = 35+APDelays[j];
            } else {
                R->APs[i][j]->buflen = APDelays[j];
            }
            R->APs[i][j]->ind = 0;
            R->APs[i][j]->buffer = (float*)calloc(sizeof(float), R->APs[i][j]->buflen);
            if (R->APs[i][j]->buffer == NULL) {
                printf("All-pass filter buffer allocation failed!\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return R;
}
