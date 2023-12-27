/*
  ==============================================================================

    chorus.cpp
    Created: 15 Jun 2023 5:30:27pm
    Author:  Anthony Hong

  ==============================================================================
*/

#include "chorus.h"

Chorus::Chorus(double sampleRate): delayBuffer(2, std::vector<float>((int)(sampleRate*10))), writePtrs(2), phases(2) {
    lfoFrequency = 0.009f;
    phases[0] = 0.0;
    phases[1] = 0.25;
    sr = sampleRate;
}

void Chorus::applyChorus(float *data, int channel, int numSamplesToProcess, float delaySamples, float wetMix, int bufLen) {
    for (int s = 0; s < numSamplesToProcess; s++) {
        float input = data[s];
        float currDelay = delaySamples*(0.5+0.5*sinf(2.0*M_PI*phases[channel]));
        float index = fmodf((float)writePtrs[channel]-(float)(currDelay*sr)+(float)bufLen-3.0f, (float)bufLen);
        float percentage = index-floorf(index);
        int prev = (int)floorf(index);
        int next = (prev+1)%bufLen;
        float interpolation = percentage*delayBuffer[channel][next]+(1.0f-percentage)*delayBuffer[channel][prev];
        delayBuffer[channel][writePtrs[channel]] = input;
        if (writePtrs[channel]++ >= bufLen)
            writePtrs[channel] = 0;
        data[s] += wetMix*interpolation;
        phases[channel] += lfoFrequency/sr;
        if (phases[channel] >= 1.0)
            phases[channel] -= 1.0;
    }
}
