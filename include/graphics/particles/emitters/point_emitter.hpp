#pragma once

#include "base_emitter.hpp"
#include <glm/glm.hpp>

namespace graphics::particles {

/// Emitter that emits particle from a single point.
class PointEmitter : public BaseEmitter {
public:
  PointEmitter(std::shared_ptr<Initializer> initializer,
               GameObject *const parent = nullptr,
               const glm::vec3 &position = glm::vec3(0.0f))
      : BaseEmitter(initializer, parent) {
    setPosition(position);
  }
  virtual ~PointEmitter() = default;

  /// Emit a particle from a specific point
  inline virtual void emit(Particle &particle) override {
    BaseEmitter::emit(particle);
    // Particle is already positioned at the emission point by BaseEmitter
  }

  /// Set emission position
  void setPosition(const glm::vec3 &pos) { position = pos; }

  /// Get emission position
  const glm::vec3 &getPosition() const { return position; }
};
} // namespace graphics::particles
