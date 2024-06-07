/** @file smasm/arguments.hpp */

#pragma once

#include <smasm/common.hpp>

namespace smasm
{

  class arguments
  {
  public:
    static bool               parse (int argc, char** argv);
    static bool               has (const std::string& key);
    static bool               has (const std::string& key, const char short_form);
    static const std::string& get (const std::string& key);
    static const std::string& get (const std::string& key, const char short_form);

  private:
    static std::unordered_map<std::string, std::string> s_args;

  };

}
