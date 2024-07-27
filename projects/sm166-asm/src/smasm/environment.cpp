/** @file smasm/environment.cpp */

#include <smasm/environment.hpp>

namespace smasm
{

  environment::environment (environment* parent, environment_scope scope) :
    m_parent { parent },
    m_scope { scope }
  {
    if (m_parent == nullptr)
    {
      m_scope = environment_scope::parent;
      create_global_env();
    }
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

      if (key.starts_with("_") == false)
      {    
        std::cerr << "[environment] Could not resolve variable name '" << key << "'." << std::endl;
      }
      return nullptr;
    }

    return it->second;
  }
  
  environment* environment::get_function_scope ()
  {
    if (m_scope == environment_scope::function)
    {
      return this;
    }
    else if (m_parent != nullptr)
    {
      return m_parent->get_function_scope();
    }
    
    return nullptr;
  };
  
  bool environment::shift_arguments (const std::uint64_t count)
  {
    if (m_scope != environment_scope::function)
    {
      return false;
    }
    
    auto old_argc = resolve_variable("_count");
    if (old_argc == nullptr || old_argc->get_value_type() != value_type::number)
    {
      return false;
    }
    
    if (
      declare_variable(
        "_count", value::make<number_value>(
          value_cast<number_value>(old_argc)->get_integer() - count)
      ) == false
    )
    {
      return false;
    }
    
    for (std::uint64_t i = 0; i < count; ++i)
    {
      std::uint64_t index = 1;
      while (true)
      {
        auto src = resolve_variable("_" + std::to_string(index));
        if (src == nullptr)
        {
          break;
        }
        
        if (declare_variable("_" + std::to_string(index - 1), src) == false)
        {
          return false;
        }
        
        index++;
      }
    }
    
    return true;
  }

  void environment::create_global_env ()
  {
    declare_variable("true",  value::make<number_value>(1), true);
    declare_variable("false", value::make<number_value>(0), true);
  }

}
