#pragma once

#include <cstdint>
#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <cstddef>
#include <cassert>
#include <optional>

namespace graphics {

/// Maximum number of point lights in the UBO (match GLSL array size)
constexpr size_t MAX_POINT_LIGHTS = 16;

/// Point light structure for UBO (std140 layout)
struct alignas(16) PointLight {
  /// Position in world space (w unused)
  alignas(16) glm::vec4 position{0.0f, 0.0f, 0.0f, 1.0f};

  /// Ambient color (w unused)
  alignas(16) glm::vec4 ambient{0.2f, 0.2f, 0.2f, 1.0f};

  /// Diffuse color (w unused)
  alignas(16) glm::vec4 diffuse{0.8f, 0.8f, 0.8f, 1.0f};

  /// Specular color (w unused)
  alignas(16) glm::vec4 specular{1.0f, 1.0f, 1.0f, 1.0f};

  /// Attenuation constant
  alignas(4) float constant = 1.0f;
  /// Attenuation linear coefficient
  alignas(4) float linear = 0.09f;
  /// Attenuation quadratic coefficient
  alignas(4) float quadratic = 0.032f;

  /// Padding to make struct size multiple of 16
  alignas(4) float __padding = 0.0f;

  PointLight() = default;

  PointLight(const glm::vec3 &position, const glm::vec3 &ambient,
             const glm::vec3 &diffuse, const glm::vec3 &specular,
             float constant = 1.0f, float linear = 0.09f,
             float quadratic = 0.032f)
      : position{position, 1.0f}, ambient{ambient, 1.0f},
        diffuse{diffuse, 1.0f}, specular{specular, 1.0f}, constant{constant},
        linear{linear}, quadratic{quadratic} {}
};

static_assert(sizeof(PointLight) == 80,
              "PointLight size mismatch with std140 layout");
static_assert(alignof(PointLight) == 16, "PointLight alignment mismatch");

/// Lighting data structure for UBO (std140 layout)
///
/// Contains all lights in the scene and camera position.
/// This is uploaded to GPU once per frame.
struct alignas(16) LightingData {
  /// Camera position in world space (w component unused)
  alignas(16) glm::vec4 viewPos{0.0f, 0.0f, 0.0f, 1.0f};

  /// Array of point lights
  alignas(16) PointLight pointLights[MAX_POINT_LIGHTS];

  /// Number of active point lights
  alignas(4) uint32_t numPointLights = 0;

  /// Padding to make struct size multiple of 16
  alignas(4) uint32_t __padding[3] = {0, 0, 0};
};

static_assert(sizeof(LightingData) == 1312,
              "LightingData size mismatch with std140 layout");
static_assert(alignof(LightingData) == 16, "LightingData alignment mismatch");

/// Manages all lights in the scene using Uniform Buffer Objects (UBO)
///
/// Lighting data is accessed through UBO binding point 1 in shaders
class LightManager {
public:
  LightManager();
  ~LightManager();

  LightManager(const LightManager &) = delete;
  LightManager &operator=(const LightManager &) = delete;

  /// Initialize LightManager and create UBO
  bool init();

  void cleanup();

  /// Add a point light to the scene
  /// Returns the index, nullopt if reached limit
  std::optional<uint32_t> addPointLight(const PointLight &light);

  /// Remove a point light by index
  void removePointLight(uint32_t index);

  /// Get mutable point light reference
  PointLight &getPointLight(uint32_t index);
  /// Get point light reference
  const PointLight &getPointLight(uint32_t index) const;

  /// Set camera position (updated each frame)
  void setViewPos(const glm::vec3 &position);

  /// Update UBO on GPU with current lighting data,
  /// called once before rendering each frame
  void updateUBO();

  /// Get the UBO binding point (constant 1)
  static constexpr GLuint getUBOBindingPoint() { return 1; }

  /// Get number of active point lights
  int numActiveLights() const { return m_numActiveLights; }

  /// Get maximum number of point lights
  static constexpr size_t getMaxPointLights() { return MAX_POINT_LIGHTS; }

  bool initialized() const { return m_ubo != 0; }

private:
  /// CPU-side lighting data (uploaded to GPU)
  LightingData m_lightingData;

  /// Number of active point lights in the array
  uint32_t m_numActiveLights = 0;

  /// OpenGL UBO
  GLuint m_ubo = 0;

  /// Flag to track if data needs updating
  bool m_dirty = true;

  /// Create and allocate UBO
  bool createUBO();
};

} // namespace graphics
