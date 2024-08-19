/** @file smasm/environment.cpp */

#include <smasm/arguments.hpp>
#include <smasm/functions.hpp>
#include <smasm/environment.hpp>

namespace smasm
{

  std::uint32_t environment::s_default_precision = 0;

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

  value::ptr environment::resolve_variable (const std::string& key, bool first_pass) const
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
        return m_parent->resolve_variable(key, first_pass);
      }

      if (key.starts_with("_") == false && first_pass == false)
      {    
        std::cerr << "[environment] Could not resolve variable name '" << key << "'." << std::endl;
      }
      return nullptr;
    }

    return it->second;
  }
  
  environment* environment::get_function_scope (bool global_counts)
  {
    if (m_scope == environment_scope::function)
    {
      return this;
    }
    else if (m_parent != nullptr)
    {
      return m_parent->get_function_scope();
    }
    
    return global_counts == true ? this : nullptr;
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
  
  value::ptr environment::operator[] (const std::string& key) const
  {
    return resolve_variable(key);
  }
  
  value::ptr environment::operator[] (const std::uint64_t index) const
  {
    return resolve_variable("_" + std::to_string(index));
  }

  void environment::create_global_env ()
  {
    declare_variable("true",      value::make<number_value>(1), true);
    declare_variable("false",     value::make<number_value>(0), true);
    declare_variable("fp_int",    value::make<function_value>(fp_int), true);
    declare_variable("fp_frac",   value::make<function_value>(fp_frac), true);
    declare_variable("fp_add",    value::make<function_value>(fp_add), true);
    declare_variable("fp_sub",    value::make<function_value>(fp_sub), true);
    declare_variable("fp_div",    value::make<function_value>(fp_div), true);
    declare_variable("fp_mul",    value::make<function_value>(fp_mul), true);
    declare_variable("fp_mod",    value::make<function_value>(fp_fmod), true);
    declare_variable("fp_pow",    value::make<function_value>(fp_pow), true);
    declare_variable("fp_log",    value::make<function_value>(fp_log), true);
    declare_variable("fp_sin",    value::make<function_value>(fp_sin), true);
    declare_variable("fp_cos",    value::make<function_value>(fp_cos), true);
    declare_variable("fp_tan",    value::make<function_value>(fp_tan), true);
    declare_variable("fp_asin",   value::make<function_value>(fp_asin), true);
    declare_variable("fp_acos",   value::make<function_value>(fp_acos), true);
    declare_variable("fp_atan",   value::make<function_value>(fp_atan), true);
  }

}
