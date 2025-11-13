#version 330 core

in vec4 particleColor;

out vec4 FragColor;

uniform sampler2D particleTexture;

void main()
{
    // Make particles circular
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(gl_PointCoord, center);

    if (dist > 0.5) {
        discard;
    }

    // Sample texture and apply color
    vec4 texColor = texture(particleTexture, gl_PointCoord);
    FragColor = texColor * particleColor;
}
