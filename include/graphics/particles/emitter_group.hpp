#pragma once

#include "particle_system.hpp"
#include <memory>
#include <numeric>
#include <vector>

namespace graphics::particles {

/// Emission policy for emitter groups
enum class EmissionPolicy {
  /// Distribute emissions across systems in order
  RoundRobin,
  /// All systems emit simultaneously
  Simultaneous,
  /// Randomly select a system for each emission
  Random,
};

/// Template class for managing multiple particle systems
/// @tparam ParticleSystemType Type of particle system to manage
template <typename ParticleSystemType = ParticleSystem<>> class EmitterGroup {
public:
  /// Constructor
  /// @param policy Emission policy for the group
  explicit EmitterGroup(EmissionPolicy policy = EmissionPolicy::RoundRobin)
      : emissionPolicy{policy} {}

  /// Add a particle system to the group
  /// @param system Particle system to add
  void addSystem(std::shared_ptr<ParticleSystemType> system) {
    systems.push_back(system);
  }

  /// Remove a particle system from the group
  /// @param system Particle system to remove
  void removeSystem(std::shared_ptr<ParticleSystemType> system) {
    auto it = std::find(systems.begin(), systems.end(), system);
    if (it != systems.end()) {
      systems.erase(it);
    }
  }

  /// Clear all systems from the group
  void clearSystems() {
    systems.clear();
    currentRoundRobinIndex = 0;
  }

  /// Get all systems in the group
  /// @return Vector of particle systems
  const std::vector<std::shared_ptr<ParticleSystemType>> &getSystems() const {
    return systems;
  }

  /// Get a specific system by index
  /// @param index Index of the system
  /// @return Particle system at index, or nullptr if invalid
  std::shared_ptr<ParticleSystemType> getSystem(size_t index) const {
    if (index < systems.size()) {
      return systems[index];
    }
    return nullptr;
  }

  /// Get number of systems in the group
  /// @return System count
  size_t getSystemCount() const { return systems.size(); }

  /// Set emission policy
  /// @param policy New emission policy
  void setEmissionPolicy(EmissionPolicy policy) { emissionPolicy = policy; }

  /// Get current emission policy
  /// @return Current emission policy
  EmissionPolicy getEmissionPolicy() const { return emissionPolicy; }

  /// Update all systems in the group
  /// @param deltaTime Time since last update
  void update(float deltaTime) {
    for (auto &system : systems) {
      if (system) {
        system->update(deltaTime);
      }
    }
  }

  /// Render all systems in the group
  /// @param view View matrix
  /// @param projection Projection matrix
  void render(const glm::mat4 &view, const glm::mat4 &projection) {
    for (const auto &system : systems) {
      if (system) {
        system->render(view, projection);
      }
    }
  }

  /// Emit a burst of particles across the group
  /// @param count Total number of particles to emit
  void emitBurst(int count) {
    if (systems.empty() || count <= 0) {
      return;
    }

    switch (emissionPolicy) {
    case EmissionPolicy::RoundRobin:
      emitRoundRobin(count);
      break;
    case EmissionPolicy::Simultaneous:
      emitSimultaneous(count);
      break;
    case EmissionPolicy::Random:
      emitRandom(count);
      break;
    }
  }

  /// Start continuous emission across the group
  /// @param particlesPerSecond Total emission rate across all systems
  void startContinuousEmission(float particlesPerSecond) {
    if (systems.empty() || particlesPerSecond <= 0.0f) {
      return;
    }

    // Calculate emission rate per system based on policy
    switch (emissionPolicy) {
    case EmissionPolicy::RoundRobin:
    case EmissionPolicy::Random:
      // Distribute rate across systems
      for (auto &system : systems) {
        if (system && system->getEmitter()) {
          system->getEmitter()->setEmissionRate(particlesPerSecond /
                                                systems.size());
        }
      }
      break;
    case EmissionPolicy::Simultaneous:
      // Each system gets full rate
      for (auto &system : systems) {
        if (system && system->getEmitter()) {
          system->getEmitter()->setEmissionRate(particlesPerSecond);
        }
      }
      break;
    }

    // Enable all systems
    for (auto &system : systems) {
      if (system) {
        system->toggle(true);
      }
    }
  }

  /// Stop continuous emission
  void stopContinuousEmission() {
    for (auto &system : systems) {
      if (system) {
        system->toggle(false);
        if (system->getEmitter()) {
          system->getEmitter()->setEmissionRate(0.0f);
        }
      }
    }
  }

  /// Toggle all systems on/off
  /// @param enabled True to enable, false to disable
  void toggle(bool enabled) {
    for (auto &system : systems) {
      if (system) {
        system->toggle(enabled);
      }
    }
  }

  /// Reset all systems
  void reset() {
    for (auto &system : systems) {
      if (system) {
        system->reset();
      }
    }
    currentRoundRobinIndex = 0;
  }

  /// Get total active particle count across all systems
  /// @return Total active particles
  size_t getTotalActiveParticles() const {
    size_t total = std::accumulate(
        systems.begin(), systems.end(), 0ZU,
        [](size_t acc, std::shared_ptr<ParticleSystemType> system) {
          return acc + (system ? system->getActiveParticleCount() : 0ZU);
        });
    return total;
  }

  /// Apply a function to all systems
  /// @param func Function to apply (takes std::shared_ptr<ParticleSystemType>)
  template <typename Func> void forEachSystem(Func func) {
    for (auto &system : systems) {
      if (system) {
        func(system);
      }
    }
  }

private:
  std::vector<std::shared_ptr<ParticleSystemType>> systems;
  EmissionPolicy emissionPolicy{EmissionPolicy::RoundRobin};
  size_t currentRoundRobinIndex{0};

  void emitForEachSystem(
      std::function<int(size_t index)> calculateCountFromSystemIndex) {
    for (const auto &[i, system] : std::ranges::views::enumerate(systems)) {
      if (system and system->getEmitter()) {
        int systemCount = calculateCountFromSystemIndex(i);
        if (systemCount > 0) {
          system->getEmitter()->setBurst(systemCount, 0.0f); // Immediate burst
        }
      }
    }
  }

  /// Emit using round-robin policy
  void emitRoundRobin(int count) {
    // Distribute count across systems
    int particlesPerSystem = count / systems.size();
    int remainder = count % systems.size();

    emitForEachSystem([&particlesPerSystem, &remainder](size_t index) {
      return particlesPerSystem + (index < remainder ? 1 : 0);
    });
  }

  /// Emit simultaneously from all systems
  void emitSimultaneous(int count) {
    // Each system emits the full count
    for (auto &system : systems) {
      if (system && system->getEmitter()) {
        system->getEmitter()->setBurst(count, 0.0f); // Immediate burst
      }
    }
  }

  /// Emit using random selection
  /// @param count Total particles to emit
  void emitRandom(int count) {
    // Count particles per system
    std::vector<int> systemCounts(systems.size(), 0);

    for (int i = 0; i < count; ++i) {
      if (systems.empty()) {
        break;
      }
      // Uniform random selection
      const size_t index = rand() % systems.size();
      systemCounts[index]++;
    }

    emitForEachSystem(
        [&systemCounts](size_t index) { return systemCounts[index]; });
  }
};
} // namespace graphics::particles
