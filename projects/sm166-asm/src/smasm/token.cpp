/** @file smasm/token.cpp */

#include <smasm/token.hpp>

namespace smasm
{

  const char* token::get_string_type () const
  {
    switch (type)
    {
      case token_type::unknown: return "unknown";
      case token_type::language: return "language";
      case token_type::directive: return "directive";
      case token_type::section: return "section";
      case token_type::condition: return "condition";
      case token_type::vector: return "vector";
      case token_type::cpu_register: return "cpu register";
      case token_type::instruction: return "instruction";
      case token_type::identifier: return "identifier";
      case token_type::string: return "string";
      case token_type::integer: return "integer";
      case token_type::binary: return "binary";
      case token_type::hexadecimal: return "hexadecimal";
      case token_type::octal: return "octal";
      case token_type::number: return "number";
      case token_type::backtick: return "backtick";
      case token_type::question: return "question";
      case token_type::exclaim: return "exclaim";
      case token_type::period: return "period";
      case token_type::comma: return "comma";
      case token_type::colon: return "colon";
      case token_type::semicolon: return "semicolon";
      case token_type::at: return "at";
      case token_type::pound: return "pound";
      case token_type::dollar: return "dollar";
      case token_type::percent: return "percent";
      case token_type::carat: return "carat";
      case token_type::pipe: return "pipe";
      case token_type::ampersand: return "ampersand";
      case token_type::asterisk: return "asterisk";
      case token_type::plus: return "plus";
      case token_type::equals: return "equals";
      case token_type::minus: return "minus";
      case token_type::backslash: return "backslash";
      case token_type::slash: return "slash";
      case token_type::open_paren: return "open paren";
      case token_type::close_paren: return "close paren";
      case token_type::open_bracket: return "open bracket";
      case token_type::close_bracket: return "close bracket";
      case token_type::open_brace: return "open brace";
      case token_type::close_brace: return "close brace";
      case token_type::open_arrow: return "open arrow";
      case token_type::close_arrow: return "close arrow";
      case token_type::new_line: return "new line";
      case token_type::end_of_file: return "end of file";
      default: return "unknown";
    }
  }

  bool token::is_integer () const
  {
    return (
      type == token_type::integer ||
      type == token_type::binary ||
      type == token_type::hexadecimal ||
      type == token_type::octal
    );
  }

  std::uint64_t token::get_integer () const
  {
    switch (type)
    {
      case token_type::integer:       return std::stoul(contents, nullptr, 10);
      case token_type::binary:        return std::stoul(contents, nullptr, 2);
      case token_type::hexadecimal:   return std::stoul(contents, nullptr, 16);
      case token_type::octal:         return std::stoul(contents, nullptr, 8);
      default:                        return 0;
    }
  }

  double token::get_number () const
  {
    switch (type)
    {
      case token_type::integer:
      case token_type::binary:
      case token_type::hexadecimal:
      case token_type::octal:         return static_cast<double>(get_integer());
      case token_type::number:        return std::stod(contents, nullptr);
      default:                        return 0.0f;
    }
  }

  const keyword& token::get_keyword () const
  {
    return keyword::lookup(contents);
  }

}
