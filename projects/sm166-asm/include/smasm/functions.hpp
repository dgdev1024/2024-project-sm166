/// @file smasm/functions.hpp

#pragma once
#include <smasm/values.hpp>

namespace smasm
{
  value::ptr fp_int (const environment& env);
  value::ptr fp_frac (const environment& env);
  value::ptr fp_add (const environment& env);
  value::ptr fp_sub (const environment& env);
  value::ptr fp_div (const environment& env);
  value::ptr fp_mul (const environment& env);
  value::ptr fp_fmod (const environment& env);
  value::ptr fp_pow (const environment& env);
  value::ptr fp_log (const environment& env);
  value::ptr fp_sin (const environment& env);
  value::ptr fp_cos (const environment& env);
  value::ptr fp_tan (const environment& env);
  value::ptr fp_asin (const environment& env);
  value::ptr fp_acos (const environment& env);
  value::ptr fp_atan (const environment& env);
}
