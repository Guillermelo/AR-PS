#include "audioAnalysis.h"
#include <cmath>
#include <vector>
#include <iostream>

static int g_fftSize = 1024;
static float g_sampleRate = 44100.0f;

void initAudioAnalysis(int fftSize) {
    g_fftSize = fftSize;
}

// Calcular volumen RMS
float computeRMS(const std::vector<float>& samples) {
    float sum = 0.0f;
    for (float s : samples) sum += s * s;
    return std::sqrt(sum / samples.size());
}











