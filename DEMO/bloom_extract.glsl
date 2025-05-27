#version 330 core
in vec2 TexCoords;
out vec3 BrightColor;

uniform sampler2D scene;

void main()
{
    vec3 color = texture(scene, TexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722)); // luminancia
    if (brightness > 0.02)
        BrightColor = color;
    else
        BrightColor = vec3(0.0);
}
