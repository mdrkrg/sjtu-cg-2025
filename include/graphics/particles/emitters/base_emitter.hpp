#pragma once

#include "graphics/particles/base/emitter.hpp"
#include "graphics/particles/initializers/base_initializer.hpp"
#include <glm/glm.hpp>

namespace graphics::particles {

class BaseEmitter : public Emitter {
public:
  BaseEmitter(std::shared_ptr<Initializer> initializer = nullptr,
              const GameObject *parent = nullptr,
              SimulationSpace space = SimulationSpace::WORLD)
      // default
      : Emitter(initializer ? initializer : std::make_shared<BaseInitializer>(),
                0.0f, parent, space) {}
  virtual ~BaseEmitter() = default;

  /// Emit a particle with emitter-specific positioning
  virtual void emit(Particle &particle) override {
    Emitter::emit(particle);

    // Get emission origin (local coordinates)
    glm::vec3 localPos = getEmissionOrigin();

    // Transform based on simulation space
    particle.position = calculatePositionBySimulationSpace(localPos);
  }

  /// Get emission offset relative to the origin
  glm::vec3 getEmissionOrigin() const { return position; }

protected:
  // Position and direction for emission
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 direction = glm::vec3(0.0f, 1.0f, 0.0f);
};
} // namespace graphics::particles
