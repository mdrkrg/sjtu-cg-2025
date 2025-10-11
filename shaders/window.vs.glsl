#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 texScale = vec2(1.0, 1.0);
uniform vec2 texOffset = vec2(0.0, 0.0);

void main()
{
    // 1. translate to texture center
    // 2. scale (reversed)
    // 3. translate back
    // 4. apply offset (reversed)
    TexCoords = ((aTexCoords - vec2(0.5, 0.5)) / texScale) + vec2(0.5, 0.5) - texOffset;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPos = vec3(model * vec4(aPos, 1.0));

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}


