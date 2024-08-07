/** @file smasm/common.hpp */

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <functional>
#include <memory>
#include <cmath>
#include <cctype>
#include <sm/common.hpp>

namespace fs = std::filesystem;

#define indent(c) std::string(c, ' ')

namespace smasm
{

  using string_list = std::vector<std::string>;

}
