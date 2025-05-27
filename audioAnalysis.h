#pragma once
#ifndef AUDIO_ANALYSIS_H
#define AUDIO_ANALYSIS_H

#include <vector>

// Inicializar (si hace falta)
void initAudioAnalysis(int fftSize);

// Acceso opcional a energía total (RMS)
float computeRMS(const std::vector<float>& samples);

#endif
