/*
  ==============================================================================

    PerlinNoise.h
    Created: 3 Feb 2025 8:56:15am
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class PerlinNoise {
public:
    static float noise(float x, float y, float z);

private:
    static float fade(float t);
    static float lerp(float t, float a, float b);
    static float grad(int hash, float x, float y, float z);
    static const int p[512];
};
