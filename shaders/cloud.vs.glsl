#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float sliceDepth;
uniform vec3 cameraPos;

void main()
{
  // World position of the cloud center
  vec3 cloudCenter = vec3(model[3]);

  // Extract scale from the model matrix by getting the length of its basis vectors
  float scaleX = length(vec3(model[0]));
  float scaleY = length(vec3(model[1]));

  // Calculate view direction from cloud center to camera
  vec3 viewDir = normalize(cameraPos - cloudCenter);

  // Billboard orientation that always faces the camera
  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = normalize(cross(up, viewDir));
  up = normalize(cross(viewDir, right));

  // Position this slice along the view direction
  vec3 sliceOffset = viewDir * sliceDepth;
  vec3 slicePos = cloudCenter + sliceOffset;

  // Orient the quad to face the camera
  vec3 worldPos = slicePos + scaleX * aPos.x * right * 0.5 + scaleY * aPos.y * up * 0.5;

  FragPos = worldPos;
  gl_Position = projection * view * vec4(worldPos, 1.0);
  TexCoord = aTexCoord;
}
