/** @file smasm/environment.hpp */

#pragma once

#include <smasm/values.hpp>

namespace smasm
{

  class environment
  {
  public:
    environment (environment* parent = nullptr);

  public:
    bool declare_variable (const std::string& key, const value::ptr& value, bool constant = false);
    value::ptr resolve_variable (const std::string& key) const;

  private:
    environment* m_parent = nullptr;
    std::unordered_map<std::string, value::ptr> m_variables;
    std::unordered_set<std::string> m_constants;

  };

}
