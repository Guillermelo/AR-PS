#pragma once
#include <glm/glm.hpp>

struct Particle {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 velocity;
    float life;
};
