#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>
#include <functional>
#include <thread>
#include <crtdbg.h>
#include <cmath>


// Global delta time
std::chrono::steady_clock::time_point lastFrameTime = std::chrono::steady_clock::now();

#include "ParticleBehavior.h"
#include "Particle.h"
#include "audioManager.h"
#include "audioThread.h" 
#include "ParticleBehavior.h"


GLuint shaderProgram;
GLuint vao, vbo;
GLuint vboPos;
GLuint quadVBO, instanceVBO;
GLuint colorVBO;


// Framebuffers y texturas para postprocesado
GLuint sceneFBO, colorTex, brightTex;
GLuint pingpongFBO[2], pingpongTex[2];
GLuint bloomShaderExtract, blurShader, compositeShader;
GLuint screenQuadVAO, screenQuadVBO;


float angle = 0.0f;
std::vector<std::function<void()>> activeBehaviors;
std::vector<Particle> particles;

std::string loadShaderSource(const char* filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return shader;
}
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}


const int numParticles = 500000;
const float maxRadius = 2.5f;


static float silenceTimer = 0.0f;
const float silenceThreshold = 0.05f;
const float silenceRequired = 5.0f;

void updateActiveBehavior_sound(float deltaTime) {
    float vol = getAudioVolume();
    resetTriggers();
    vol = vol * 10;
    if (vol < silenceThreshold) {
        silenceTimer += deltaTime;
    }
    else {
        silenceTimer = 0.0f;  // se rompe el silencio, se reinicia
    }

    if (silenceTimer >= silenceRequired) {
        activeBehaviors.push_back([=]() {
            behavior_wave_terrain_particles(particles, angle, deltaTime);
            });
    }
    else {
        // No hacer nada. Si querés debug:
        // std::cout << "⏳ Esperando silencio... " << silenceTimer << "s\n";
    }





    if ((vol) > 0.32f) {
        
        activeBehaviors.push_back([=]() {
        behavior_vortex_particles(particles, angle, deltaTime);
        });
        activeBehaviors.push_back([=]() {
            behavior_lightning_ray_particles(particles, angle, deltaTime);
            });

    }
    if ((vol > 0.14f)&&(vol <= 0.32f)) {
        
        activeBehaviors.push_back([=]() {
            //glm::vec3 explosionOrigin = glm::vec3(0.0f);
            //glm::vec3 explosionDirection = glm::vec3(1.0f, 6.0f, 0.0f); // explosión hacia X
            alltozero(particles, angle, deltaTime);
            });
        activeBehaviors.push_back([=]() {
            //glm::vec3 explosionOrigin = glm::vec3(0.0f);
            //glm::vec3 explosionDirection = glm::vec3(1.0f, 6.0f, 0.0f); // explosión hacia X
            behavior_attract_random_clusters_particles(particles, angle, deltaTime,vol);
            });
    }

    if ((vol > 0.009f) && (vol <= 0.14f)) {
        activeBehaviors.push_back([=]() {
            
            glm::vec3 explosionOrigin = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 explosionOrigin2 = glm::vec3(0.0f, 0.1f, 1.0f);
            glm::vec3 explosionOrigin3 = glm::vec3(0.0f, 0.1f, -1.0f);
            glm::vec3 explosionOrigin4 = glm::vec3(1.0f, 0.1f, 0.0f);
            glm::vec3 explosionOrigin5 = glm::vec3(-1.0f, 0.1f, 0.0f);
            float radius = 1.6f;
            float strength = 2.f;
            //behavior_force_field(particles, explosionOrigin, 4.f, 0.009f,deltaTime);
            behavior_force_field(particles, explosionOrigin2, radius, strength, deltaTime);
            behavior_force_field(particles, explosionOrigin3, radius, strength, deltaTime);
            behavior_force_field(particles, explosionOrigin4, radius, -0.5f, deltaTime);
            behavior_force_field(particles, explosionOrigin5, radius, -0.5f, deltaTime);
            });
    }

}

void addBehavior(float deltaTime)
{
    /*activeBehaviors.push_back([=]() {
        behavior_vortex_particles(particles, angle, deltaTime);
        });*/
        
}

void PlayActiveBehavior()
{
    try {
        for (auto& x : activeBehaviors) {
            x();
        }
    }
    catch (...) {
        std::cerr << "[ERROR] Excepción durante ejecución de behavior. Posible lambda inválido." << std::endl;
    }
}


void setupBloomBuffers(int width, int height) {
    // Framebuffer de escena con dos salidas
    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    // Color (escena base)
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    // Color brillante
    glGenTextures(1, &brightTex);
    glBindTexture(GL_TEXTURE_2D, brightTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, brightTex, 0);

    // Renderbuffer de profundidad
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[FBO] Incompleto!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Ping-pong blur
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongTex);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongTex[i], 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}





void init() 
{
    shaderProgram = createShaderProgram("vertex.glsl", "fragment.glsl");
    bloomShaderExtract = createShaderProgram("quad.glsl", "bloom_extract.glsl");
    blurShader = createShaderProgram("quad.glsl", "blur.glsl");
    compositeShader = createShaderProgram("quad.glsl", "bloom_composite.glsl");

    // Quad fullscreen para postprocesado
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);
    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    float quad[] = {
        // x     y     z
        -0.5f, -0.5f, 0.0f, 
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,

        -0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };

    // creación de posiciones y colores
    for (int i = 0; i < numParticles; ++i) {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float radius = sqrt(static_cast<float>(rand()) / RAND_MAX) * maxRadius;
        float x = radius * cos(angle);
        float y = (rand() % 2001 - 1000) / 1000000.f;
        float z = radius * sin(angle);

        Particle p;
        p.position = glm::vec3(x, y, z);
        p.color = glm::vec3(0.5f); // gris claro, solo algunas brillarán

        p.life = 5.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f;
        particles.push_back(p);
    }


    particles.resize(numParticles);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // VBO de geometría (cuad)
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // VBO de posiciones
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * particles.size(), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(1, 1);

    // VBO de colores (nuevo)
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * particles.size(), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
}

void display() {
    float size = 0.0005f;
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastFrameTime).count();
    lastFrameTime = now;

    // === Paso 1: Render al framebuffer con múltiples targets ===
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    activeBehaviors.clear();
    addBehavior(deltaTime);
    updateActiveBehavior_sound(deltaTime);
    PlayActiveBehavior();

    // Validación y buffers
    auto isVec3Finite = [](const glm::vec3& v) { return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z); };
    std::vector<glm::vec3> positions(particles.size());
    std::vector<glm::vec3> colors(particles.size());
    for (size_t i = 0; i < particles.size(); ++i) {
        positions[i] = isVec3Finite(particles[i].position) ? particles[i].position : glm::vec3(0);
        colors[i] = isVec3Finite(particles[i].color) ? particles[i].color : glm::vec3(1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * positions.size(), positions.data());
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * colors.size(), colors.data());

    GLint sizeLoc = glGetUniformLocation(shaderProgram, "particleSize");
    glUniform1f(sizeLoc, size);

    float radius = 2.8f;
    float camX = radius * sin(angle);
    float camZ = radius * cos(angle);
    glm::vec3 cameraPos = glm::vec3(camX, 0.01f, camZ);
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::perspective(glm::radians(50.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 vp = proj * view;

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "vp"), 1, GL_FALSE, &vp[0][0]);

    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particles.size());
    glBindVertexArray(0);

    // === Paso 2: Blur en ping-pong ===
    bool horizontal = true, first = true;
    glUseProgram(blurShader);
    for (int i = 0; i < 10; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        glUniform1i(glGetUniformLocation(blurShader, "horizontal"), horizontal);
        glBindTexture(GL_TEXTURE_2D, first ? brightTex : pingpongTex[!horizontal]);
        glBindVertexArray(screenQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizontal = !horizontal;
        if (first) first = false;
    }

    // === Paso 3: Composición final ===
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(compositeShader);
    glUniform1i(glGetUniformLocation(compositeShader, "scene"), 0);
    glUniform1i(glGetUniformLocation(compositeShader, "bloomBlur"), 1);
    glUniform1i(glGetUniformLocation(compositeShader, "bloom"), true);
    glUniform1f(glGetUniformLocation(compositeShader, "exposure"), 1.2f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongTex[!horizontal]);
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    angle += 0.0001f;
    glutSwapBuffers();
}







void onClose() {
    stopAudioProcessingThread();
}



int main(int argc, char** argv) 
{
    // audio section
    #if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif

    
    
    
    // lo de siempre
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
	glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("particles");

    glewInit();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    init();
    setupBloomBuffers(screenWidth, screenHeight);


    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutCloseFunc(onClose);

    std::cout << "[MAIN] Lanzando hilo de audio..." << std::endl;
    startAudioProcessingThread();

    glutMainLoop();
    
    return 0;
}
