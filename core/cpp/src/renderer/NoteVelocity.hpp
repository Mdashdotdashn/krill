#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <optional>
#include <string>

namespace krill
{
  namespace note_velocity
  {
    inline constexpr int kMin = 0;
    inline constexpr int kMax = 127;
    inline constexpr int kDefault = 127;

    inline int clampMidiVelocity(int value)
    {
      return std::clamp(value, kMin, kMax);
    }

    inline std::optional<double> parseVelocityNumber(const std::string& text)
    {
      try
      {
        size_t consumed = 0;
        const double parsed = std::stod(text, &consumed);
        if (consumed != text.size())
        {
          return std::nullopt;
        }
        return parsed;
      }
      catch (...)
      {
        return std::nullopt;
      }
    }

    inline int resolveVelocityToMidi(double value)
    {
      const double absoluteValue = std::fabs(value);
      const double scaledValue = (value >= 0.0 && value <= 1.0)
        ? (absoluteValue * static_cast<double>(kDefault))
        : absoluteValue;
      const auto roundedValue = static_cast<int>(std::llround(scaledValue));
      return clampMidiVelocity(roundedValue);
    }

    inline int multiplyMidiVelocities(int left, int right)
    {
      const auto combined = static_cast<int>(std::llround(
        (static_cast<double>(resolveVelocityToMidi(static_cast<double>(left)))
         * static_cast<double>(resolveVelocityToMidi(static_cast<double>(right))))
        / static_cast<double>(kDefault)));
      return clampMidiVelocity(combined);
    }

    inline std::optional<int> resolveControlVelocityToMidi(
      const std::map<std::string, std::string>& controls,
      const std::string& key)
    {
      const auto found = controls.find(key);
      if (found == controls.end())
      {
        return std::nullopt;
      }

      const auto parsed = parseVelocityNumber(found->second);
      if (!parsed.has_value())
      {
        return std::nullopt;
      }

      return resolveVelocityToMidi(parsed.value());
    }
  }
}