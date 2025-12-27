#version 330 core

// Vertex attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// Instance attributes (mat4 takes 4 attribute locations)
layout (location = 3) in mat4 instanceMatrix;

// Output to fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

// Uniforms
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Apply instance transformation
    mat4 model = instanceMatrix;

    // Transform position
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);

    // Transform normal (using normal matrix)
    // The normal matrix is transpose(inverse(model)) but we only need the 3x3 part
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * aNormal;

    // Pass through texture coordinates
    TexCoords = aTexCoords;

    // Final position
    gl_Position = projection * view * worldPos;
}