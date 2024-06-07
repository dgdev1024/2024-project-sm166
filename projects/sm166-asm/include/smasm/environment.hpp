/** @file smasm/environment.hpp */

#pragma once

#include <smasm/values.hpp>

namespace smasm
{

  class environment
  {
  public:
    bool declare_variable (const std::string& key, const value::ptr& value);
    value::ptr resolve_variable (const std::string& key) const;

  private:
    std::unordered_map<std::string, value::ptr> m_variables;

  };

}
