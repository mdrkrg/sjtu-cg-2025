#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D windowTexture;
uniform vec3 objectColor;

void main()
{
    vec4 texSample = texture(windowTexture, TexCoords);

    // Alpha ~1.0: window frame
    // Alpha ~0.2: wall color
    // Alpha ~0.0: transparent
    if (texSample.a > 0.9)
    {
        FragColor = vec4(texSample.rgb, 1.0);
    }
    else if (texSample.a > 0.1 && texSample.a < 0.3)
    {
        FragColor = vec4(objectColor, 1.0);
    }
    else
    {
        discard;
    }
}
