/** @file smboy/arguments.cpp */

#include <smboy/arguments.hpp>

namespace smboy
{

  static const std::string blank_string = "";
  std::unordered_map<std::string, std::string> arguments::s_args;

  bool arguments::parse (int argc, char** argv)
  {
    if (argc > 20) {
      std::cerr <<  "[arguments::parse] "
                <<  "Too many arguments (" << argc - 1 << ") passed in."
                <<  std::endl;
      return false;
    }
    
    // Iterate over the command-line arguments, starting at index 1.
    for (int index = 1; index < argc; ++index) {

      // Get the current argument.
      std::string argument = argv[index];

      // A long-form command-line argument begins with two dashes ('--'). Check for that, first.
      if (argument.starts_with("--") == true) {

        // Keep a key-value pair. Also, check for an equals sign between them.
        std::string key = "", value = "";
        std::size_t equals_sign_pos = argument.find('=');

        if (equals_sign_pos != std::string::npos) {
          key   = argument.substr(2, equals_sign_pos - 2);
          value = argument.substr(equals_sign_pos + 1);

          s_args[key] = value;
        } else if (index + 1 < argc && argv[index + 1][0] != '-') {
          key   = argument.substr(2);
          s_args[key] = argv[++index];
        } else {
          key   = argument.substr(2);
          s_args[key] = "true";
        }

      }

      // A short-form argument is a single letter preceeded by a single dash ('-').
      else if (argument.starts_with("-") == true) {

        std::string key = argument.substr(1);
        if (index + 1 < argc && argv[index + 1][0] != '-') {
          s_args[key] = argv[++index];
        } else {
          s_args[key] = "true";
        }

      }

    }

    return true;
  }

  bool arguments::has (const std::string& key)
  {
    return s_args.contains(key);
  }

  bool arguments::has (const std::string& key, const char short_form)
  {
    return s_args.contains(key) || s_args.contains({ short_form });
  }

  const std::string& arguments::get (const std::string& key)
  {
    auto it = s_args.find(key);
    if (it != s_args.end()) {
      return it->second;
    }

    return blank_string;
  }

  const std::string& arguments::get (const std::string& key, const char short_form)
  {
    auto lit = s_args.find(key);
    auto sit = s_args.find({ short_form });

    if (lit != s_args.end()) {
      return lit->second;
    } else if (sit != s_args.end()) {
      return sit->second;
    }

    return blank_string;
  }

}
