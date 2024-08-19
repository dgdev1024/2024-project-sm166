/// @file smasm/functions.cpp

#include <smasm/environment.hpp>
#include <smasm/functions.hpp>

namespace smasm
{

  #define smasm_expect_arg(index, type) \
    auto arg##index = env[index - 1]; \
    if (arg##index == nullptr) \
    { \
      std::cerr << "[" << __FUNCTION__ << "] Missing argument #" << index \
                << " to function call." << std::endl; \
      return nullptr; \
    } \
    else if (arg##index->get_value_type() != value_type::type) \
    { \
      std::cerr << "[" << __FUNCTION__ << "] Expected argument #" << index \
                << " to function call to be of type '" << #type << "'." \
                << std::endl; \
      return nullptr; \
    }

  #define smasm_optional_arg(index, type) \
    auto arg##index = env[index - 1]; \
    if ( \
      arg##index != nullptr && \
      arg##index->get_value_type() != value_type::type \
    ) \
    { \
      std::cerr << "[" << __FUNCTION__ << "] Expected argument #" << index \
                << " to function call to be of type '" << #type << "'." \
                << std::endl; \
      return nullptr; \
    }

  /* Fixed Point Functions ************************************************************************/

  value::ptr fp_int (const environment& env)
  {
    smasm_expect_arg(1, number)
    return value::make<number_value>(
      value_cast<number_value>(arg1)->get_integer()
    );
  }

  value::ptr fp_frac (const environment& env)
  {
    smasm_expect_arg(1, number)
    return value::make<number_value>(
      value_cast<number_value>(arg1)->get_fractional()
    );
  }

  value::ptr fp_add (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_expect_arg(2, number)
    smasm_optional_arg(3, number)

    auto num1 = value_cast<number_value>(arg1)->get_number();
    auto num2 = value_cast<number_value>(arg2)->get_number();

    double integer = 0.0;
    double fractional = std::modf(num1 + num2, &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg3 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg3)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_sub (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_expect_arg(2, number)
    smasm_optional_arg(3, number)

    auto num1 = value_cast<number_value>(arg1)->get_number();
    auto num2 = value_cast<number_value>(arg2)->get_number();

    double integer = 0.0;
    double fractional = std::modf(num1 - num2, &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg3 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg3)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_div (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_expect_arg(2, number)
    smasm_optional_arg(3, number)

    auto num1 = value_cast<number_value>(arg1)->get_number();
    auto num2 = value_cast<number_value>(arg2)->get_number();
    
    if (num2 == 0.0)
    {
      std::cerr << "[fp_div] Attempted division by zero encountered."
                << std::endl;
      return nullptr;
    }

    double integer = 0.0;
    double fractional = std::modf(num1 / num2, &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg3 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg3)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_mul (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_expect_arg(2, number)
    smasm_optional_arg(3, number)

    auto num1 = value_cast<number_value>(arg1)->get_number();
    auto num2 = value_cast<number_value>(arg2)->get_number();

    double integer = 0.0;
    double fractional = std::modf(num1 * num2, &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg3 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg3)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_fmod (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_expect_arg(2, number)
    smasm_optional_arg(3, number)

    auto num1 = value_cast<number_value>(arg1)->get_number();
    auto num2 = value_cast<number_value>(arg2)->get_number();
    
    if (num2 == 0.0)
    {
      std::cerr << "[fp_fmod] Modulo with attempted division by zero encountered."
                << std::endl;
      return nullptr;
    }

    double integer = 0.0;
    double fractional = std::modf(std::fmod(num1, num2), &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg3 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg3)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_pow (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_expect_arg(2, number)
    smasm_optional_arg(3, number)

    auto num1 = value_cast<number_value>(arg1)->get_number();
    auto num2 = value_cast<number_value>(arg2)->get_number();

    double integer = 0.0;
    double fractional = std::modf(std::pow(num1, num2), &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg3 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg3)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_log (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_expect_arg(2, number)
    smasm_optional_arg(3, number)

    auto num1 = value_cast<number_value>(arg1)->get_number();
    auto num2 = value_cast<number_value>(arg2)->get_number();
    std::cout << "[fp_log] " << num1 << " " << num2 << std::endl;

    if (num1 <= 0.0 || num2 <= 0.0)
    {
      std::cerr << "[fp_log] Logarithm base and operand must be greater than zero."
                << std::endl;
      return nullptr;
    }

    double integer = 0.0;
    double fractional = std::modf(
      std::log(num1) / std::log(num2), &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg3 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg3)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_sin (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_optional_arg(2, number)
    auto num1 = value_cast<number_value>(arg1)->get_number();
    
    double integer = 0.0;
    double fractional = std::modf(std::sin(num1 * (2 * M_PI)), &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg2 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg2)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_cos (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_optional_arg(2, number)
    auto num1 = value_cast<number_value>(arg1)->get_number();
    
    double integer = 0.0;
    double fractional = std::modf(std::cos(num1 * (2 * M_PI)), &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg2 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg2)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_tan (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_optional_arg(2, number)
    auto num1 = value_cast<number_value>(arg1)->get_number();
    
    double integer = 0.0;
    double fractional = std::modf(std::tan(num1 * (2 * M_PI)), &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg2 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg2)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_asin (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_optional_arg(2, number)
    auto num1 = value_cast<number_value>(arg1)->get_number();
    
    double integer = 0.0;
    double fractional = std::modf(std::asin(num1 * (2 * M_PI)), &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg2 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg2)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_acos (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_optional_arg(2, number)
    auto num1 = value_cast<number_value>(arg1)->get_number();
    
    double integer = 0.0;
    double fractional = std::modf(std::acos(num1 * (2 * M_PI)), &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg2 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg2)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

  value::ptr fp_atan (const environment& env)
  {
    smasm_expect_arg(1, number)
    smasm_optional_arg(2, number)
    auto num1 = value_cast<number_value>(arg1)->get_number();
    
    double integer = 0.0;
    double fractional = std::modf(std::atan(num1 * (2 * M_PI)), &integer);
              
    std::uint8_t fraction_bits = default_fraction_bits;
    if (arg2 != nullptr)
    {
      fraction_bits = value_cast<number_value>(arg2)->get_integer();
    }

    return value::make<number_value>(
      static_cast<std::uint64_t>(integer),
      fractional,
      fraction_bits
    );
  }

}
