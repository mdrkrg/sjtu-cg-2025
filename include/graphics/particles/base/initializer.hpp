#pragma once
#include "graphics/particles/particle.hpp"

namespace graphics::particles {

/// Interface of the initializer of particles, owned by particle emitter
class Initializer {

public:
  /// Initialize a particle with initializer-specific properties
  virtual void initialize(Particle &particle) = 0;
};
} // namespace graphics::particles
