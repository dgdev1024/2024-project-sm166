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
  
  value::ptr environment::operator[] (const std::string& key) const
  {
    return resolve_variable(key);
  }
  
  value::ptr environment::operator[] (const std::uint64_t index) const
  {
    return resolve_variable("_" + std::to_string(index));
  }

  namespace functions
  {
    
    value::ptr round (const environment& env)
    {
      const auto& val_one = env[0];
      
      if (val_one == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (val_one->get_value_type() != value_type::number) {
        return value::make<void_value>();
      }
      
      return value::make<number_value>(
        std::round(
          value_cast<number_value>(val_one)->get_number()
        )
      );
    }
    
    value::ptr ceil (const environment& env)
    {
      const auto& val_one = env[0];
      
      if (val_one == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (val_one->get_value_type() != value_type::number) {
        return value::make<void_value>();
      }
      
      return value::make<number_value>(
        std::ceil(
          value_cast<number_value>(val_one)->get_number()
        )
      );
    }
    
    value::ptr floor (const environment& env)
    {
      const auto& val_one = env[0];
      
      if (val_one == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (val_one->get_value_type() != value_type::number) {
        return value::make<void_value>();
      }
      
      return value::make<number_value>(
        std::floor(
          value_cast<number_value>(val_one)->get_number()
        )
      );
    }
  
    value::ptr integral (const environment& env)
    {
      const auto& val_one = env[0];
      
      if (val_one == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (val_one->get_value_type() != value_type::number) {
        return value::make<void_value>();
      }
      
      double int_ = 0.0;
      modf(value_cast<number_value>(val_one)->get_number(), &int_);
      return value::make<number_value>(int_);
    }
    
    value::ptr fractional (const environment& env)
    {
      const auto& val_one = env[0];
      
      if (val_one == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (val_one->get_value_type() != value_type::number) {
        return value::make<void_value>();
      }
      
      double int_ = 0.0;
      double frac_ = modf(value_cast<number_value>(val_one)->get_number(), &int_);
      
      std::string str = std::to_string(frac_);
      std::size_t period_pos = str.find('.');
      
      if (period_pos != std::string::npos)
      {
        while (str.ends_with('0') && str.ends_with(".0") == false) { str.pop_back(); }
        
        std::uint64_t f = std::stoul(str.substr(period_pos + 1), nullptr, 10);
        return value::make<number_value>(f);
      }
      else
      {
        return value::make<number_value>(frac_);
      }
    }
    
    value::ptr fmod (const environment& env)
    {
      const auto& val_one = env[0];
      const auto& val_two = env[1];
      
      if (val_one == nullptr || val_two == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (
        val_one->get_value_type() != value_type::number ||
        val_two->get_value_type() != value_type::number
      ) {
        return value::make<void_value>();
      }
      
      return value::make<number_value>(
        std::fmod(
          value_cast<number_value>(val_one)->get_number(),
          value_cast<number_value>(val_two)->get_number()
        )
      );
    }
  
    value::ptr pow (const environment& env)
    {
      const auto& val_one = env[0];
      const auto& val_two = env[1];
      
      if (val_one == nullptr || val_two == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (
        val_one->get_value_type() != value_type::number ||
        val_two->get_value_type() != value_type::number
      ) {
        return value::make<void_value>();
      }
      
      return value::make<number_value>(
        std::pow(
          value_cast<number_value>(val_one)->get_number(),
          value_cast<number_value>(val_two)->get_number()
        )
      );
    }
  
    value::ptr log (const environment& env)
    {
      const auto& val_one = env[0];
      const auto& val_two = env[1];
      
      if (val_one == nullptr || val_two == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (
        val_one->get_value_type() != value_type::number ||
        val_two->get_value_type() != value_type::number
      ) {
        return value::make<void_value>();
      }
      
      auto operand  = value_cast<number_value>(val_one)->get_number();
      auto base     = value_cast<number_value>(val_two)->get_number();
      
      if (base <= 0.0 || operand <= 0.0)
      {
        return value::make<void_value>();
      }
      
      return value::make<number_value>(
        std::log(operand) / std::log(base)
      );
    }
    
    value::ptr sine (const environment& env)
    {
    
      const auto& val_one = env[0];
      
      double turns = 0.0;
      
      if (val_one == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (val_one->get_value_type() != value_type::number) {
        return value::make<void_value>();
      }
      
      turns = value_cast<number_value>(val_one)->get_number();
      
      return value::make<number_value>(std::sin(turns * (2 * M_PI)));
    
    }
    
    value::ptr cosine (const environment& env)
    {
    
      const auto& val_one = env[0];
      
      double turns = 0.0;
      
      if (val_one == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (val_one->get_value_type() != value_type::number) {
        return value::make<void_value>();
      }
      
      turns = value_cast<number_value>(val_one)->get_number();
      
      return value::make<number_value>(std::cos(turns * (2 * M_PI)));
    
    }
    
    value::ptr tangent (const environment& env)
    {
    
      const auto& val_one = env[0];
      
      double turns = 0.0;
      
      if (val_one == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (val_one->get_value_type() != value_type::number) {
        return value::make<void_value>();
      }
      
      turns = value_cast<number_value>(val_one)->get_number();
      
      return value::make<number_value>(std::tan(turns * (2 * M_PI)));
    
    }
    
    value::ptr str_length (const environment& env)
    {
      const auto& val_one = env[0];
      
      if (val_one == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (val_one->get_value_type() != value_type::string) {
        return value::make<void_value>();
      }
      
      return value::make<number_value>(
        value_cast<string_value>(val_one)->get_string().length()
      );
    }
    
    value::ptr str_compare (const environment& env)
    {
      const auto& val_one = env[0];
      const auto& val_two = env[1];
      
      if (val_one == nullptr || val_two == nullptr)
      {
        return value::make<void_value>();
      }
      
      if (
        val_one->get_value_type() != value_type::string ||
        val_two->get_value_type() != value_type::string
      ) {
        return value::make<void_value>();
      }
      
      auto result = value_cast<string_value>(val_one)->get_string().compare(
        value_cast<string_value>(val_two)->get_string()
      );  
      
      if (result < 0) { result = -1; }
      else if (result > 0) { result = 1; }
      
      return value::make<number_value>(result);
    }
    
  }

  void environment::create_global_env ()
  {
    declare_variable("true",  value::make<number_value>(1), true);
    declare_variable("false", value::make<number_value>(0), true);
    
    declare_variable("round", value::make<function_value>(functions::round), true);
    declare_variable("ceil",  value::make<function_value>(functions::ceil), true);
    declare_variable("floor", value::make<function_value>(functions::floor), true);
    declare_variable("int",   value::make<function_value>(functions::integral), true);
    declare_variable("frac",  value::make<function_value>(functions::fractional), true);
    declare_variable("fmod",  value::make<function_value>(functions::fmod), true);
    declare_variable("pow",   value::make<function_value>(functions::pow), true);
    declare_variable("log",   value::make<function_value>(functions::log), true);
    declare_variable("sin",   value::make<function_value>(functions::sine), true);
    declare_variable("cos",   value::make<function_value>(functions::cosine), true);
    declare_variable("tan",   value::make<function_value>(functions::tangent), true);
    
    declare_variable("strlen",  value::make<function_value>(functions::str_length), true);
    declare_variable("strcmp",  value::make<function_value>(functions::str_compare), true);
  }

}
