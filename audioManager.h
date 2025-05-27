#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <vector>

// Inicialización y limpieza
bool initAudio();
void closeAudio();

// Llamado por el thread para actualizar estado
void updateAudio();

// Acceso al buffer de samples capturados
// (copia los últimos N samples a un buffer externo)
void getLatestSamples(std::vector<float>& outBuffer, int numSamples);

// También podés seguir usando volumen si querés
float getCurrentVolume();

#endif
