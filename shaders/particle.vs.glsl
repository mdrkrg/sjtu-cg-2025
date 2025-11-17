#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aSize;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

uniform float scaleFactor = 10.0;

out vec4 particleColor;

void main()
{
    particleColor = aColor;

    vec4 worldPos = model * vec4(aPos, 1.0);
    float distanceToCamera = distance(worldPos.xyz, viewPos);

    gl_Position = projection * view * worldPos;

    gl_PointSize = aSize * (scaleFactor / distanceToCamera);
}
