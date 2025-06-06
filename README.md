# Real-Time Audio Reactive Particle System (C++ / OpenGL)

This project is an interactive visual particle system that responds in real-time to live audio input. It is built using **C++**, **OpenGL**, and **PortAudio**, and includes various audio-reactive particle behaviors enhanced with **post-processing bloom effects**.

## Demo

> 📽️ Watch the demo on YouTube: [https://youtu.be/OrttOR6UHtk](https://youtu.be/OrttOR6UHtk)

<p align="center">
  <img src="./images/2025-05-2702-20-45-ezgif.com-video-to-gif-converter (2).gif" width="70%" />
</p>

### Screenshots

<p align="center">
  <img src="./images/Screenshot 2025-05-27 094620.png" width="45%" />
  <img src="./images/Screenshot 2025-05-27 094744.png" width="45%" />
</p>
<p align="center">
  <img src="./images/Screenshot 2025-05-27 094938.png" width="45%" />
  <img src="./images/Screenshot 2025-05-27 094948.png" width="45%" />
</p>

## Features

- Real-time audio capture using **PortAudio**
- Fast particle updates using instancing (up to 500,000 particles)
- Audio-reactive behaviors:
  - **Wave terrain**
  - **Electric rays**
  - **Vortex spiral**
  - **Cluster attraction**
  - **Force fields**
- Visual effects with **HDR bloom and Gaussian blur**
- Multi-threaded audio analysis to prevent blocking the render loop

---

## Build Instructions

### Dependencies

Make sure to install or link the following:

- OpenGL 3.3+
- FreeGLUT
- GLEW
- GLM
- PortAudio

### Build Example (Windows + Visual Studio)

1. Add all `.cpp` and `.h` files to a Visual Studio project.
2. Link against `portaudio_x86.lib` or `portaudio_x64.lib` depending on your system.
3. Make sure `glew32.lib`, `freeglut.lib`, `opengl32.lib`, and `glm` are linked.
4. Set working directory to the folder where your GLSL shader files are (e.g. `vertex.glsl`, `fragment.glsl`, etc.).
5. Build and run.

---


## File Overview

| File | Purpose |
|------|---------|
| `CG.cpp` | Main rendering loop, OpenGL setup, behavior dispatch |
| `audioManager.cpp/.h` | Audio input using PortAudio |
| `audioThread.cpp/.h` | Background thread for audio analysis |
| `audioAnalysis.cpp/.h` | Computes RMS (volume) |
| `ParticleBehavior.h` | Contains all particle behavior logic |
| `vertex.glsl`, `fragment.glsl`, `bloom_extract.glsl`, `blur.glsl`, `bloom_composite.glsl`, `quad.glsl` | Shader code |

---

## Notes

- The system uses RMS (root mean square) for basic energy analysis.
- Additional particle behaviors are modular and easily extendable.
- Audio trigger thresholds are configurable from the source code.

---
