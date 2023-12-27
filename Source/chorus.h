/*
  ==============================================================================

    chorus.h
    Created: 15 Jun 2023 5:30:32pm
    Author:  Anthony Hong

  ==============================================================================
*/

#pragma once
#include <vector>
#include <cmath>
#include <iostream>

class Chorus {
public:
    Chorus(double sampleRate);
    void applyChorus(float *data, int channel, int numSamplesToProcess, float delaySamples, float wetMix, int bufLen);
private:
    std::vector<std::vector<float>> delayBuffer;
    std::vector<int> writePtrs;
    std::vector<float> phases;
    float lfoFrequency;
    double sr;
};
