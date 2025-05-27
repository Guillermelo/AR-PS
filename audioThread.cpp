#include "audioThread.h"
#include "audioManager.h"
#include "audioAnalysis.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <iostream>

static std::thread audioThread;
static std::atomic<bool> running(false);

// Estados compartidos
static std::atomic<float> currentVolume(0.0f);
static std::atomic<float> currentFrequency(0.0f);
static std::atomic<bool> bassTrigger(false);
static std::atomic<bool> pitchTrigger(false);

// Parámetros
constexpr int FFT_SIZE = 1024;
constexpr float SAMPLE_RATE = 44100.0f;
constexpr float BASS_THRESHOLD = 0.2f;
constexpr float PITCH_FREQ_MIN = 400.0f; // ejemplo: activar si detecta algo entre 400-500 Hz
constexpr float PITCH_FREQ_MAX = 500.0f;

void audioLoop() {
    //std::cout << "[AUDIO] Llamando a initAudio()..." << std::endl;
    if (!initAudio()) {
        std::cerr << "[AUDIO] Falló initAudio(), abortando hilo de audio." << std::endl;
        return;
    }
    
    initAudioAnalysis(FFT_SIZE);

    while (running) {
        //std::cout << "[AUDIO] Entrando al loop..." << std::endl;

        std::vector<float> samples;
        getLatestSamples(samples, FFT_SIZE);

        // ✅ Protección contra heap corruption
        if (samples.size() < FFT_SIZE) {
            //std::cout << "[AUDIO] Aún no hay suficientes samples. Esperando..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        float rms = computeRMS(samples);
        //std::cout << "[AUDIO LOOP] RMS: " << rms << std::endl;

        currentVolume.store(rms);

        // Triggers por volumen
        if (rms > BASS_THRESHOLD) {
            bassTrigger.store(true);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60Hz
    }
}


void startAudioProcessingThread() {
    running = true;
    audioThread = std::thread(audioLoop);
}

void stopAudioProcessingThread() {
    running = false;
    if (audioThread.joinable()) {
        audioThread.join();
    }
}

float getAudioVolume() {
    return currentVolume.load();
}


bool isBassTriggered() {
    return bassTrigger.load();
}


void resetTriggers() {
    bassTrigger.store(false);
    pitchTrigger.store(false);
}
