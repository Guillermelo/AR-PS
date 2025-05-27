#version 330 core
in vec2 TexCoords;
out vec3 FragColor;

uniform sampler2D image;
uniform bool horizontal;

const float weight[9] = float[](
    0.05, 0.09, 0.12, 0.15, 0.18, 0.15, 0.12, 0.09, 0.05
    );

void main()
{
    vec2 tex_offset = 1.0 / textureSize(image, 0); // tamaño del texel
    vec3 result = vec3(0.0);

    for (int i = -4; i <= 4; ++i)
    {
        vec2 offset = horizontal ? vec2(tex_offset.x * i, 0.0) : vec2(0.0, tex_offset.y * i);
        result += texture(image, TexCoords + offset).rgb * weight[i + 4];
    }

    FragColor = result;
}
