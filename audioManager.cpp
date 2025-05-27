#include "audioManager.h"
#include <portaudio.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <cmath>
#include <cstring>

#include <atomic>
static std::atomic<int> writeIndex(0);

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256
#define AUDIO_BUFFER_SIZE (SAMPLE_RATE * 2) // 2 segundos de audio

static float audioBuffer[AUDIO_BUFFER_SIZE];
static std::mutex bufferMutex;

static float currentVolume = 0.0f;

static PaStream* stream = nullptr;

static int audioCallback(const void* inputBuffer, void*, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*) {
    const float* input = static_cast<const float*>(inputBuffer);
    if (!input) return paContinue;

    std::lock_guard<std::mutex> lock(bufferMutex);

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        float sample = input[i];
        int currentIndex = writeIndex.fetch_add(1) % AUDIO_BUFFER_SIZE;
        audioBuffer[currentIndex] = sample;
    }

    return paContinue;
}

bool initAudio() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "Error al inicializar PortAudio: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    PaStreamParameters inputParams;
    //inputParams.device = 5; // INPUT INDEX IN MY CASE THE FOCUSTIRE IS 5
    //if (inputParams.device == paNoDevice) {
    //    std::cerr << "No hay dispositivo de entrada." << std::endl;
    //    return false;
    //}


    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        std::cerr << "Error al obtener dispositivos: " << Pa_GetErrorText(numDevices) << std::endl;
        return false;
    }

    int inputDeviceIndex = -1;
    for (int i = 0; i < numDevices; ++i) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo && deviceInfo->maxInputChannels > 0) {
            inputDeviceIndex = i;
            std::cout << "[Audio] Usando dispositivo de entrada: " << deviceInfo->name << std::endl;
            break;
        }
    }

    if (inputDeviceIndex == -1) {
        std::cerr << "No se encontró un dispositivo de entrada válido." << std::endl;
        return false;
    }

    inputParams.device = inputDeviceIndex;

    inputParams.channelCount = 1;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(&stream, &inputParams, nullptr, SAMPLE_RATE,
        FRAMES_PER_BUFFER, paClipOff, audioCallback, nullptr);
    if (err != paNoError) {
        std::cerr << "Error al abrir el stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Error al iniciar el stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    std::cout << "AudioManager iniciado." << std::endl;
    return true;
}

void closeAudio() {
    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        stream = nullptr;
    }
    Pa_Terminate();
}

float getCurrentVolume() {
    return currentVolume;
}

void updateAudio() {
    // Si necesitás hacer algo activo, podés usarlo.
}

void getLatestSamples(std::vector<float>& outBuffer, int numSamples) {
    std::lock_guard<std::mutex> lock(bufferMutex);

    int currentWriteIndex = writeIndex.load();  // acceso atómico seguro

    if (currentWriteIndex < numSamples) {
        outBuffer.clear(); // no hay suficientes muestras aún
        return;
    }

    outBuffer.resize(numSamples);
    int startIdx = (currentWriteIndex - numSamples + AUDIO_BUFFER_SIZE) % AUDIO_BUFFER_SIZE;

    for (int i = 0; i < numSamples; ++i) {
        int index = (startIdx + i) % AUDIO_BUFFER_SIZE;
        outBuffer[i] = audioBuffer[index];
    }

    // std::cout << "[SAMPLES] Muestras copiadas correctamente: " << numSamples << std::endl;
}
