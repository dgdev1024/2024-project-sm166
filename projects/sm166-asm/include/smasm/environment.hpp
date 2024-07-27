/** @file smasm/environment.hpp */

#pragma once

#include <smasm/values.hpp>

namespace smasm
{

  enum class environment_scope
  {
    parent,
    function,
    repeat,
    if_statement
  };

  class environment
  {
  public:
    environment (environment* parent = nullptr, 
      environment_scope scope = environment_scope::parent);

  public:
    bool declare_variable (const std::string& key, const value::ptr& value, bool constant = false);
    value::ptr resolve_variable (const std::string& key) const;
    environment* get_function_scope ();
    bool shift_arguments (const std::uint64_t count);
    
  public:
    value::ptr operator[] (const std::string& key) const;
    value::ptr operator[] (const std::uint64_t index) const;

  private:
    void create_global_env ();

  public:
    inline environment_scope get_scope () const { return m_scope; }

  private:
    environment* m_parent = nullptr;
    environment_scope m_scope = environment_scope::parent;
    std::unordered_map<std::string, value::ptr> m_variables;
    std::unordered_set<std::string> m_constants;

  };

}
