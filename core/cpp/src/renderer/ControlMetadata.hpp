#pragma once

#include <map>
#include <optional>
#include <stdexcept>
#include <string>

#include "NoteVelocity.hpp"

namespace krill
{
  namespace control_metadata
  {
    using Controls = std::map<std::string, std::string>;

    inline Controls canonicalizeModelControls(Controls controls, bool isNestedSource)
    {
      if (!isNestedSource)
      {
        return controls;
      }

      if (controls.count("velocity") > 0)
      {
        const auto velocity = note_velocity::resolveControlVelocityFactor(controls, "velocity");
        const auto factor = note_velocity::resolveControlVelocityFactor(controls, "velocityFactor");

        if (!velocity.has_value())
        {
          throw std::invalid_argument("Nested velocity must be within [0, 1].");
        }

        controls["velocityFactor"] = factor.has_value()
          ? note_velocity::formatVelocityFactor(note_velocity::accumulateVelocityFactor(factor.value(), velocity.value()))
          : note_velocity::formatVelocityFactor(velocity.value());
        controls.erase("velocity");
      }

      if (controls.count("velocityFactor") > 0)
      {
        const auto factor = note_velocity::resolveControlVelocityFactor(controls, "velocityFactor");
        if (!factor.has_value())
        {
          throw std::invalid_argument("Nested velocityFactor must be within [0, 1].");
        }
        controls["velocityFactor"] = note_velocity::formatVelocityFactor(factor.value());
      }

      return controls;
    }

    inline Controls validateNestedControls(Controls controls)
    {
      if (controls.count("velocity") > 0)
      {
        throw std::invalid_argument("Nested ElementRenderNode controls must use velocityFactor, not velocity.");
      }

      if (controls.count("velocityFactor") > 0)
      {
        const auto factor = note_velocity::resolveControlVelocityFactor(controls, "velocityFactor");
        if (!factor.has_value())
        {
          throw std::invalid_argument("Nested ElementRenderNode velocityFactor must be within [0, 1].");
        }
        controls["velocityFactor"] = note_velocity::formatVelocityFactor(factor.value());
      }

      return controls;
    }

    inline Controls composeNestedControls(const Controls& parentControls, const Controls& childControls)
    {
      Controls mergedControls = parentControls;
      for (const auto& entry : childControls)
      {
        mergedControls[entry.first] = entry.second;
      }

      const auto parentFactor = note_velocity::resolveControlVelocityFactor(parentControls, "velocityFactor");
      const auto childFactor = note_velocity::resolveControlVelocityFactor(childControls, "velocityFactor");
      std::optional<double> accumulatedFactor;

      const auto accumulate = [&accumulatedFactor](const std::optional<double>& contribution)
      {
        if (!contribution.has_value())
        {
          return;
        }

        accumulatedFactor = accumulatedFactor.has_value()
          ? note_velocity::accumulateVelocityFactor(accumulatedFactor.value(), contribution.value())
          : contribution.value();
      };

      accumulate(parentFactor);
      accumulate(childFactor);

      if (accumulatedFactor.has_value())
      {
        mergedControls["velocityFactor"] = note_velocity::formatVelocityFactor(accumulatedFactor.value());
      }

      return mergedControls;
    }
  }
}
