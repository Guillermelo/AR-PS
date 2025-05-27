#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <cmath>
#include <cstdlib>
#include "Particle.h"


inline void respawnIfDead(Particle& p)
{
    if (p.life <= 0.0f) {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265f;
        float radius = sqrt(static_cast<float>(rand()) / RAND_MAX) * 3.5f;

        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float y = ((rand() % 2001) / 10000.0f) - 0.1f;

        p.position = glm::vec3(x, y, z);
        p.life = 2.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f;
    }
}

inline void respawnInVortex(Particle& p)
{
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
    float radius = sqrt(static_cast<float>(rand()) / RAND_MAX) * 2.0f;
    float x = radius * cos(angle);
    float z = radius * sin(angle);
    float y = ((rand() % 2001) / 10000.0f) - 0.1f;

    p.position = glm::vec3(x, y, z);
    p.color = glm::vec3(0.3f, 0.1f, 0.9f);
    p.life = 2.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f;
}

// 🌪️ Mini vortex espacial
inline void behavior_vortex_particles(std::vector<Particle>& particles, float time, float deltaTime)
{
    for (auto& p : particles)
    {
        float angle = 0.001f * time;
        float x = p.position.x;
        float z = p.position.z;

        float newX = -z * sin(angle) * 0.002f * deltaTime;
        float newZ = x * cos(angle) * 0.002f * deltaTime;

        p.position.x += newX;
        p.position.z += newZ;

        p.color.r = 0.3f + 0.3f * sin(time * 0.5f + x);
        p.color.g = 0.1f + 0.2f * cos(time * 0.3f + z);
        p.color.b = 0.6f + 0.4f * sin(time * 0.7f + x * z);
        p.life -= 0.8f * deltaTime;
        respawnIfDead(p);
    }
}

// 💥 Mini explosiones espaciales


// ⚡ Pequeñas descargas eléctricas espaciales
inline void behavior_lightning_ray_particles(std::vector<Particle>& particles, float time, float deltaTime)
{
    glm::vec3 targetCenter = glm::vec3(0.0f, 0.0f, 0.0f); // Centro del rayo

    for (auto& p : particles)
    {
        // Dirección desde la partícula hacia el centro del "rayo"
        glm::vec3 dir = glm::normalize(glm::vec3(targetCenter.x - p.position.x, 0.0f, targetCenter.z - p.position.z));

        // Movimiento acumulativo en XZ hacia el centro (con leve oscilación para "chispa")
        float jitter = 0.1f * sin(time * 10.0f + p.position.x * 5.0f);
        p.position.x += dir.x * deltaTime * 5.0f + jitter * deltaTime;
        p.position.z += dir.z * deltaTime * 5.0f + jitter * deltaTime;

        // Y permanece fija
        p.position.y = 0.0f;

        // Color tipo descarga eléctrica espacial
        p.color.r = 0.6f + 0.3f * sin(time * 5.0f + p.position.x);
        p.color.g = 0.8f + 0.2f * sin(time * 3.0f + p.position.z);
        p.color.b = 1.0f;

        p.life -= deltaTime;
        respawnIfDead(p);
    }
}


// 🌫️ Movimiento hacia arriba con oscilación vertical

inline void behavior_attract_random_clusters_particles(std::vector<Particle>& particles, float time, float deltaTime, float vol)
{
    static std::vector<glm::vec3> attractors;
    static bool initialized = false;

    // Initialize 9 random attractors inside a 2.5f radius sphere (once)
    if (!initialized) {
        for (int i = 0; i < 15; ++i) {
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265f; // angle in radians
            float radius = static_cast<float>(rand()) / RAND_MAX * .5f;              // distance from center
            float x = radius * cos(angle);  // polar to cartesian
            float z = radius * sin(angle);
            float y = ((rand() % 1001) / 1000.0f) - 0.5f;  // y ∈ [-0.5, 0.5]
            attractors.push_back(glm::vec3(x, y, z));
        }
        initialized = true;
    }

    // Speed of attraction depends on volume (scaled and clamped)
    float speedFactor = glm::clamp(vol * 5.0f, 0.1f, 1.0f);

    // Loop over each particle
    for (auto& p : particles)
    {
        // Pick a random attractor
        const glm::vec3& target = attractors[rand() % attractors.size()];
        glm::vec3 dir = target - p.position;
        float dist = glm::length(dir);

        // Apply attraction force if not too close
        if (dist > 0.01f) {
            dir = glm::normalize(dir);
            float force = 0.4f / (dist * dist + 0.01f);  // inverse-square law
            p.position += dir * force * deltaTime * 60.0f * speedFactor;
        }

        // Dynamic color based on position and time (spacey effect)
        p.color.r = 0.4f + 0.5f * sin(time + p.position.x);
        p.color.g = 0.2f + 0.4f * cos(time + p.position.z * 1.5f);
        p.color.b = 0.8f + 0.2f * sin(time + p.position.y);

        // (Optional) could decrement life and respawn here
        // p.life -= deltaTime;
        // respawnIfDead(p);
    }
}





inline void behavior_force_field(std::vector<Particle>& particles, glm::vec3 center, float radius, float strength, float deltaTime)
{
    for (auto& p : particles)
    {
        glm::vec3 dir = p.position - center;
        float dist = glm::length(dir);

        if (dist < radius && dist > 0.001f) {
            glm::vec3 forceDir = glm::normalize(dir);
            // Para succión, hacé: forceDir = -forceDir;

            float attenuation = 1.0f - (dist / radius); // más fuerza cerca del centro
            glm::vec3 force = forceDir * strength * attenuation;

            // Aplicar fuerza a la posición como pseudo-aceleración
            p.position += force * deltaTime;

            // Color opcional por efecto
            //p.color = glm::mix(p.color, glm::vec3(1.0f, 0.3f, 0.1f), attenuation);
        }

        p.life -= 0.2f * deltaTime;
        respawnIfDead(p);
    }
}



inline void behavior_fluid_flow_field(std::vector<Particle>& particles, glm::vec3 center, float radius, glm::vec3 flowDir, float time, float deltaTime)
{
    flowDir = glm::normalize(flowDir);
    float baseSpeed = 3.0f;     // velocidad del flujo
    float turbulence = 3.4f;    // intensidad del zig-zag
    float waveAmp = 0.15f;
    float waveFreq = 3.0f;

    for (auto& p : particles)
    {
        glm::vec3 offset = p.position - center;
        float dist = glm::length(offset);

        if (dist < radius && dist > 0.001f)
        {
            float attenuation = 1.0f - (dist / radius);  // más fuerte en el centro

            // flujo principal
            glm::vec3 velocity = flowDir * baseSpeed;

            // turbulencia perpendicular con sinusoide
            glm::vec3 perp = glm::normalize(glm::cross(flowDir, glm::vec3(0.0f, 1.0f, 0.0f)));
            float wave = sin(time * waveFreq + glm::dot(p.position, perp) * 5.0f);
            velocity += perp * wave * waveAmp;

            // aplicar fuerza con atenuación
            p.position += velocity * deltaTime * attenuation;

            // color opcional para visualizar flujo
            p.color.r = 0.3f + 0.4f * attenuation;
            p.color.g = 0.6f + 0.3f * wave;
            p.color.b = 0.9f;

            //p.life -= 0.5f * deltaTime;
            //respawnIfDead(p);
        }
    }
}

inline void behavior_impact_field(std::vector<Particle>& particles, glm::vec3 impactCenter, float impactRadius, float maxForce, float time, float deltaTime)
{
    for (auto& p : particles)
    {
        glm::vec3 offset = p.position - impactCenter;
        float dist = glm::length(offset);

        // Solo afectamos si está en la zona del impacto
        if (dist < impactRadius && dist > 0.001f)
        {
            glm::vec3 dir = glm::normalize(offset);

            // Fuerza proporcional al centro del impacto (más fuerte cerca del centro)
            float force = maxForce * (1.0f - dist / impactRadius);

            // Solo aplicar la primera vez (ráfaga de impulso)
            if (time < 0.05f) {
                p.velocity = dir * force;
                p.color = glm::vec3(1.0f, 0.5f, 0.2f); // efecto visual tipo golpe
            }
        }

        // Si ya tenía velocidad, seguir aplicando movimiento con decaimiento
        if (glm::length(p.velocity) > 0.01f)
        {
            p.velocity *= 0.94f; // disipación de energía rápida
            p.position += p.velocity * deltaTime;

            p.life -= deltaTime;
            respawnIfDead(p);
        }
    }
}

inline void behavior_directional_impact_field(std::vector<Particle>& particles, glm::vec3 impactCenter, float impactRadius, glm::vec3 impactDirection, float maxForce, float time, float deltaTime)
{
    impactDirection = glm::normalize(impactDirection);

    for (auto& p : particles)
    {
        glm::vec3 offset = p.position - impactCenter;
        float dist = glm::length(offset);

        if (dist < impactRadius && dist > 0.001f)
        {
            // Peso de fuerza según cercanía al centro
            float force = maxForce * (1.0f - dist / impactRadius);

            // Dirección de salida = impacto con desviación radial (más orgánico)
            glm::vec3 radialOffset = glm::normalize(offset);
            glm::vec3 direction = glm::normalize(impactDirection + 0.5f * radialOffset);

            // Aplicar fuerza inicial en el momento del impacto
            if (time < 0.05f)
            {
                p.velocity = direction * force;
                p.color = glm::vec3(0.8f, 0.4f, 0.1f); // color rojizo como shock visual
            }
        }

        // Movimiento + disipación si fue impactado
        if (glm::length(p.velocity) > 0.01f)
        {
            p.velocity *= 0.94f; // disipación rápida
            p.position += p.velocity * deltaTime;

            p.life -= deltaTime;
            respawnIfDead(p);
        }
    }
}





// AUDIO BEHAVIORS

inline float waveTexture(float x, float z)
{
    return 0.8f * (
        sin(x * 3.0f) * cos(z * 2.0f) +
        cos(x * 1.5f + z * 0.8f)
        ); // entre -1 y 1
}

inline void behavior_wave_terrain_particles(std::vector<Particle>& particles, float time, float deltaTime)
{
    const float maxHeight = 0.2f;
    const float adjustSpeed = 2.0f;

    for (auto& p : particles)
    {
        // Inicialización forzada al plano Y = 0
        if (time < 0.01f) {
            p.position.y = 0.0f;
            p.velocity = glm::vec3(0.0f);
        }

        // Altura deseada según textura de onda
        float x = p.position.x;
        float z = p.position.z;

        float noise = waveTexture(x, z); // entre -1 y 1
        float targetHeight = (noise * 0.5f + 0.5f) * maxHeight; // normalizar a [0, 1] y escalar a [0, 0.2]

        // Movimiento suave hacia esa altura
        float deltaY = targetHeight - p.position.y;
        p.velocity.y = deltaY * adjustSpeed;
        p.position.y += p.velocity.y * deltaTime;

        // ⚠️ No se toca el color
        //p.life -= deltaTime;
        //respawnIfDead(p);
    }
}

inline void alltozero(std::vector<Particle>& particles, float time, float deltaTime)
{


    for (auto& p : particles)
    {
        p.position.y = 0;
        // ⚠️ No se toca el color
        p.life -= deltaTime;
        respawnIfDead(p);
    }
}
