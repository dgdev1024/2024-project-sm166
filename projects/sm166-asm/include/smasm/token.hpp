/** @file smasm/token.hpp */

#pragma once

#include <smasm/keyword.hpp>

namespace smasm
{

  enum class token_type
  {
    unknown,
    language,
    directive,
    section,
    condition,
    vector,
    cpu_register,
    instruction,
    identifier,   
    string,       
    integer,
    binary,
    hexadecimal,
    octal,
    number,   
    backtick,
    question,
    exclaim,    
    period,
    comma,
    colon,
    semicolon,
    at,
    pound,
    dollar,
    percent,
    carat,
    pipe,
    double_pipe,
    ampersand,
    double_ampersand,
    asterisk,
    equals,
    double_equals,
    not_equals,
    greater_equals,
    less_equals,
    plus,
    minus,
    backslash,
    slash,
    open_paren,
    close_paren,
    open_bracket,
    close_bracket,
    open_brace,
    close_brace,
    open_arrow,
    close_arrow,
    left_shift,
    right_shift,
    new_line,
    end_of_file
  };

  struct token
  {
    std::string     source_file = "";
    std::size_t     source_line = 0;
    token_type      type = token_type::unknown;
    std::string     contents = "";

    bool            is_integer () const;

    const char*     get_string_type () const;
    std::uint64_t   get_integer () const;
    double          get_number () const;
    const keyword&  get_keyword () const;

  };

}
