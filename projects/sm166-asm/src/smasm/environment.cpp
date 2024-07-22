/** @file smasm/environment.cpp */

#include <smasm/environment.hpp>

namespace smasm
{

  environment::environment (environment* parent) :
    m_parent { parent }
  {

  }

  bool environment::declare_variable (const std::string& key, const value::ptr& value, 
    bool constant)
  {
    if (key == "") {
      std::cerr << "[environment] Variable name is blank." << std::endl;
      return false;
    } else if (keyword::lookup(key).type != keyword_type::none) {
      std::cerr << "[environment] Variable name '" << key << "' is a reserved keyword." << std::endl;
      return false;
    } else if (m_constants.contains(key) == true) {
      std::cerr << "[environment] Variable name '" << key 
                << "' is a constant and cannot be re-declared." << std::endl;
      return false;
    }
    
    m_variables[key] = value;
    if (constant == true) {
      m_constants.insert(key);
    }

    return true;
  }

  value::ptr environment::resolve_variable (const std::string& key) const
  {
    if (key == "") {
      std::cerr << "[environment] Resolve key string is blank." << std::endl;
      return nullptr;
    } else if (keyword::lookup(key).type != keyword_type::none) {
      std::cerr << "[environment] Resolve key '" << key << "' is a reserved keyword." << std::endl;
      return nullptr;
    } 

    auto it = m_variables.find(key);
    if (it == m_variables.end()) {
      if (m_parent != nullptr)
      {
        return m_parent->resolve_variable(key);
      }

      std::cerr << "[environment] Could not resolve variable name '" << key << "'." << std::endl;
      return nullptr;
    }

    return it->second;
  }

}
