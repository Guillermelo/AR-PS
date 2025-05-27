#version 330 core

layout(location = 0) in vec3 vertexPos;       // Vertex of the quad
layout(location = 1) in vec3 instancePos;     // Particle position
layout(location = 2) in vec3 inColor;
out vec3 fragColor;

uniform mat4 vp;
uniform float particleSize;

void main()
{
    // Extract the right and up vectors from the view matrix
    vec3 right = vec3(vp[0][0], vp[1][0], vp[2][0]);
    vec3 up    = vec3(vp[0][1], vp[1][1], vp[2][1]);

    // Build a billboard quad that always faces the camera
    vec3 billboardOffset = (vertexPos.x * right + vertexPos.y * up) * particleSize;

    vec3 worldPos = instancePos + billboardOffset;
    fragColor = inColor;
    gl_Position = vp * vec4(worldPos, 1.0);
}
