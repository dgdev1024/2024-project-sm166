/** @file sm / common.hpp */

#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <functional>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#define sm_getbit(value, bit) (value & (1 << bit))
#define sm_setbit(value, bit, on) \
  if (on) { value = (value | (1 << bit)); } \
  else { value = (value & ~(1 << bit)); }

namespace sm
{

  using byte_buffer = std::vector<std::uint8_t>;

}
