#ifndef AUDIO_THREAD_H
#define AUDIO_THREAD_H

void startAudioProcessingThread();
void stopAudioProcessingThread();

// Accesibles desde OpenGL
float getAudioVolume();           // RMS actual
bool isBassTriggered();           // Trigger por volumen
void resetTriggers();

#endif
