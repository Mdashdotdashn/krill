#pragma once

#include <rapidjson/document.h>

namespace krill
{
  static bool hasMember(const rapidjson::Value& v, const std::string& member)
  {
    const auto it = v.FindMember(member.c_str());
    return it != v.MemberEnd();
  }

  template <typename T>
  T optionOrValue(const rapidjson::Value& v, const std::string& member, const T& defaultValue)
  {
    if constexpr (std::is_same_v<T, bool>)
    {
      return hasMember(v, member) ? float(v[member.c_str()].GetBool()) : defaultValue;
    }
    return hasMember(v, member) ? float(v[member.c_str()].GetDouble()) : defaultValue;
  }
} // namespace krill