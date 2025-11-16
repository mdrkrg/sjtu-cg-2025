#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

uniform sampler3D volumeTexture;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform float alpha;
uniform float sliceDepth;

void main()
{
  // Sample the volume texture
  vec3 texCoord = vec3(TexCoord, (sliceDepth + 0.5));
  float density = texture(volumeTexture, texCoord).r;

  if (density < 0.1) {
      discard;
  }

  // Simple lighting based on position and light direction
  vec3 lightDir = normalize(lightPos - FragPos);
  vec3 viewDir = normalize(cameraPos - FragPos);

  // Simple diffuse lighting
  float diff = max(dot(vec3(0.0, 1.0, 0.0), lightDir), 0.0);

  // Rim lighting for cloud edges
  float rim = 1.0 - max(dot(normalize(FragPos - cameraPos), vec3(0.0, 1.0, 0.0)), 0.0);
  rim = pow(rim, 2.0) * 0.5;

  // Combine lighting
  vec3 cloudColor = vec3(0.9, 0.9, 0.95);
  vec3 color = cloudColor * (0.5 + diff * 0.5) + rim * vec3(1.0, 1.0, 1.0);

  // Apply density
  float finalAlpha = density * alpha;

  FragColor = vec4(color, finalAlpha);
}
