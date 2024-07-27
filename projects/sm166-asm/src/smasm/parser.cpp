/** @file smasm/parser.cpp */

#include <smasm/parser.hpp>

namespace smasm
{

  program::ptr parser::parse_program (lexer& _lexer)
  {
    program::ptr _program = statement::make<program>();

    while (_lexer.has_more_tokens()) {
      auto stmt = parse_directive(_lexer);
      if (stmt == nullptr) {
        const auto& last_token = _lexer.token_at();
        std::cerr <<  "[parser]   In source file '" << last_token.source_file << "':"
                  <<  last_token.source_line << "."
                  <<  std::endl;
        return nullptr;
      } else {
        _program->push(stmt);
      }
    }

    return _program;
  }

  /** Directive Parsing Methods *******************************************************************/

  statement::ptr parser::parse_directive (lexer& _lexer)
  {
    if (_lexer.token_at().type == token_type::period) {
      _lexer.discard_token();

      auto directive_token = _lexer.discard_token();
      auto directive_keyword = directive_token.get_keyword();
      if (directive_keyword.type != keyword_type::directive) {
        std::cerr <<  "[parser] Expected directive token after '.'; "
                  <<  "Got '" << directive_token.get_string_type() << "' instead." 
                  <<  std::endl;
        return nullptr;
      }

      switch (directive_keyword.param_one)
      {
        case directive_type::dt_size:     return parse_size_directive(_lexer);
        case directive_type::dt_section:  return parse_section_directive(_lexer);
        default:
          std::cerr <<  "[parser] Un-implemented directive: '" << directive_token.contents << "'."
                    <<  std::endl;
          return nullptr;
      }
    }

    return parse_statement(_lexer);
  }

  statement::ptr parser::parse_size_directive (lexer& _lexer)
  {
    auto size_token = _lexer.discard_token();
    if (size_token.is_integer() == false) {
      std::cerr <<  "[parser] Expected integer token after '.size' directive; "
                <<  "Got '" << size_token.get_string_type() << "' instead." 
                <<  std::endl;
      return nullptr;
    }

    auto size = size_token.get_integer();
    if (size < 0x210)
    {
      std::cerr <<  "[parser] Expected minimum size of " << 0x210 << " bytes in '.size' directive; "
                <<  "Got " << size << " bytes instead." 
                <<  std::endl;
      return nullptr;
    } else if (size > 0x4000000) {
      std::cerr <<  "[parser] Expected maximum size of " << 0x4000000 << " bytes in '.size' directive; "
                <<  "Got " << size << " bytes instead." 
                <<  std::endl;
      return nullptr;
    }

    return statement::make<size_directive>(size);
  }

  statement::ptr parser::parse_section_directive (lexer& _lexer)
  {
    auto section_token = _lexer.discard_token();
    auto section_keyword = section_token.get_keyword();
    if (section_keyword.type != keyword_type::section) {
      std::cerr <<  "[parser] Expected 'rom' or 'ram' after '.section' directive; "
                <<  "Got '" << section_token.get_string_type() << "' instead." 
                <<  std::endl;
      return nullptr;
    }

    bool is_ram = (section_keyword.param_one == section_type::st_ram);
    auto address_expr = parse_expression(_lexer);
    if (address_expr == nullptr) { return nullptr; }

    return statement::make<section_directive>(is_ram, address_expr);
  }

  /** Statement Parsing Methods *******************************************************************/

  statement::ptr parser::parse_statement (lexer& _lexer)
  {
    const auto& token_kw = _lexer.token_at().get_keyword();
    if (token_kw.type == keyword_type::language)
    {
      auto lang_token = _lexer.discard_token();
      auto lang_keyword = lang_token.get_keyword();

      switch (lang_keyword.param_one)
      {
        case language_type::lt_def:     return parse_label_statement(_lexer);
        case language_type::lt_byte:    return parse_data_statement(_lexer, 1);
        case language_type::lt_word:    return parse_data_statement(_lexer, 2);
        case language_type::lt_long:    return parse_data_statement(_lexer, 4);
        case language_type::lt_repeat:  return parse_repeat_statement(_lexer);
        case language_type::lt_shift:   return parse_shift_statement(_lexer);
        case language_type::lt_include: return parse_include_statement(_lexer);
        case language_type::lt_incbin:  return parse_incbin_statement(_lexer);
        case language_type::lt_let:     return parse_variable_declaration_statement(_lexer, false);
        case language_type::lt_const:   return parse_variable_declaration_statement(_lexer, true);
        case language_type::lt_function:return parse_function_expression(_lexer);
        default:
          std::cerr <<  "[parser] Un-implemented language statement: '" << lang_token.contents << "'."
                    <<  std::endl;
          return nullptr;
      }
    } else if (token_kw.type == keyword_type::instruction) {
      return parse_instruction_statement(_lexer);
    }

    return parse_expression(_lexer);
  }

  statement::ptr parser::parse_variable_declaration_statement (lexer& _lexer, bool constant)
  {
    // bool global = (
    //   _lexer.token_at().get_keyword().type == keyword_type::language &&
    //   _lexer.token_at().get_keyword().param_one == language_type::lt_global
    // );
    // if (global == true)
    // {
    //   _lexer.discard_token();
    // }
    bool global = false;
    auto global_kw = _lexer.token_at().get_keyword();
    if (global_kw.type == keyword_type::language)
    {
      if (global_kw.param_one == language_type::lt_global)
      {
        global = true;
        _lexer.discard_token();
      }
      else if (global_kw.param_one == language_type::lt_local)
      {
        _lexer.discard_token();
      }
    }

    auto key_expr = parse_primary_expression(_lexer);
    std::string key = "";
    if (key_expr == nullptr) {
      return nullptr;
    } else if (key_expr->get_syntax_type() == syntax_type::identifier) {
      auto key_identifier = expression_cast<identifier>(key_expr);
      if (key_identifier->get_keyword().type != keyword_type::none)
      {
        std::cerr <<  "[parser] Variable key identifier '" << key_identifier->get_symbol() 
                  <<  "' is a reserved keyword."
                  <<  std::endl;
        return nullptr;
      }

      key = key_identifier->get_symbol();
    } else if (key_expr->get_syntax_type() == syntax_type::string_literal) {
      auto key_string = expression_cast<string_literal>(key_expr);
      if (keyword::lookup(key_string->get_string()).type != keyword_type::none)
      {
        std::cerr <<  "[parser] Variable key string '" << key_string->get_string()
                  <<  "' resolves to a reserved keyword."
                  <<  std::endl;
        return nullptr;
      }

      key = key_string->get_string();
    } else if (key_expr->get_syntax_type() == syntax_type::binary_expression) {
      key = "<binary expression>";
    } else {
      std::cerr <<  "[parser] Expected key in variable declaration to be a string or identifier."
                <<  std::endl;
      return nullptr;
    } 

    if (_lexer.discard_token().type != token_type::equals) 
    {
      std::cerr <<  "[parser] Expected '=' after key in declaration of variable '"
                <<  key << "'."
                <<  std::endl;
      return nullptr;
    }

    auto value_expr = parse_expression(_lexer);
    if (value_expr == nullptr) {
      return nullptr;
    }

    return statement::make<variable_declaration_statement>(key_expr, value_expr, constant, global);
  }

  statement::ptr parser::parse_label_statement (lexer& _lexer)
  {
    auto label_expr = parse_expression(_lexer);
    if (label_expr == nullptr) {
      return nullptr;
    } else if (label_expr->get_syntax_type() != syntax_type::identifier) {
      std::cerr <<  "[parser] Expected identifier expression after 'def' in label declaration."
                <<  std::endl;
      return nullptr;
    } 
    
    auto label_identifier = expression_cast<identifier>(label_expr);
    if (label_identifier->get_keyword().type != keyword_type::none)
    {
      std::cerr <<  "[parser] Label identifier '" << label_identifier->get_symbol() 
                <<  "' is a reserved keyword."
                <<  std::endl;
      return nullptr;
    }

    if (_lexer.discard_token().type != token_type::colon) 
    {
      std::cerr <<  "[parser] Expected ':' after declaration of label '"
                <<  label_identifier->get_symbol() << "'."
                <<  std::endl;
      return nullptr;
    }

    return statement::make<label_statement>(label_identifier);
  }

  statement::ptr parser::parse_data_statement (lexer& _lexer, int size)
  {
    expression::array arr;

    while (true) {
      auto expr = parse_expression(_lexer);
      if (expr == nullptr) {
        return nullptr;
      } else {
        arr.push_back(expr);
      }

      if (_lexer.token_at().type == token_type::comma) {
        _lexer.discard_token();
      } else {
        return statement::make<data_statement>(arr, size);
      }
    }
  }
  
  statement::ptr parser::parse_repeat_statement (lexer& _lexer)
  {
    expression::ptr count_expr = parse_expression(_lexer);
    if (count_expr == nullptr)
    {
      return nullptr;
    }
    
    if (_lexer.discard_token().type != token_type::open_brace)
    {
      std::cerr << "[parser] Expected '{' after count expression in repeat statement."
                << std::endl;
      return nullptr;
    }
    
    statement::body body;
    while (true)
    {
      if (_lexer.token_at().type == token_type::close_brace)
      {
        _lexer.discard_token();
        break;
      }
      
      statement::ptr stmt = parse_statement(_lexer);
      if (stmt == nullptr)
      {
        return nullptr;
      }
      
      body.push_back(stmt);
    }
    
    return statement::make<repeat_statement>(count_expr, body);
  }
  
  statement::ptr parser::parse_shift_statement (lexer& _lexer)
  {
    expression::ptr count_expr = parse_expression(_lexer);
    if (count_expr == nullptr)
    {
      return nullptr;
    }
    else if (count_expr->get_syntax_type() != syntax_type::numeric_literal)
    {
      std::cerr << "[parser] Expected numeric literal for count expression in shift statement."
                << std::endl;
      return nullptr;
    }
    
    return statement::make<shift_statement>(count_expr);
  }

  statement::ptr parser::parse_include_statement (lexer& _lexer)
  {
    auto filename_expr = parse_expression(_lexer);
    if (filename_expr == nullptr) {
      return nullptr;
    } else if (filename_expr->get_syntax_type() != syntax_type::string_literal) {
      std::cerr <<  "[parser] Expected string literal after 'include' in include statement."
                <<  std::endl;
      return nullptr;
    }
    
    return statement::make<include_statement>(
      expression_cast<string_literal>(filename_expr)
    );
  }

  statement::ptr parser::parse_incbin_statement (lexer& _lexer)
  {
    auto filename_expr = parse_expression(_lexer);
    if (filename_expr == nullptr) {
      return nullptr;
    } else if (filename_expr->get_syntax_type() != syntax_type::string_literal) {
      std::cerr <<  "[parser] Expected string literal after 'incbin' in include binary statement."
                <<  std::endl;
      return nullptr;
    }
    
    return statement::make<incbin_statement>(
      expression_cast<string_literal>(filename_expr)
    );
  }

  statement::ptr parser::parse_instruction_statement (lexer& _lexer)
  {
    auto mnemonic_token = _lexer.discard_token();
    auto mnemonic_keyword = mnemonic_token.get_keyword();

    expression::ptr first = nullptr;
    if (mnemonic_keyword.param_two >= 1) {
      first = parse_expression(_lexer);
      if (first == nullptr) { return nullptr; }
    }

    expression::ptr second = nullptr;
    if (mnemonic_keyword.param_two == 2) {
      if (_lexer.discard_token().type != token_type::comma) {
        std::cerr <<  "[parser] Expected ',' between arguments of instruction '"
                  <<  mnemonic_token.contents << "'." << std::endl;
        return nullptr;
      }

      second = parse_expression(_lexer);
      if (second == nullptr) { return nullptr; }
    }

    return statement::make<instruction_statement>(mnemonic_token.contents, first, second);
  }

  /** Expression Parsing Methods ******************************************************************/

  expression::ptr parser::parse_expression (lexer& _lexer)
  {
    const auto& token_kw = _lexer.token_at().get_keyword();
    if (token_kw.type == keyword_type::language)
    {
      auto lang_token = _lexer.discard_token();
      auto lang_keyword = lang_token.get_keyword();

      switch (lang_keyword.param_one)
      {
        case language_type::lt_function:  return parse_function_expression(_lexer);
        default:
          std::cerr <<  "[parser] Un-implemented language expression: '" 
                    << lang_token.contents << "'."
                    <<  std::endl;
          return nullptr;
      }
    }

    return parse_bitwise_expression(_lexer);
  }

  expression::ptr parser::parse_function_expression (lexer& _lexer)
  {
    bool global = false;
    auto global_kw = _lexer.token_at().get_keyword();
    if (global_kw.type == keyword_type::language)
    {
      if (global_kw.param_one == language_type::lt_global)
      {
        global = true;
        _lexer.discard_token();
      }
      else if (global_kw.param_one == language_type::lt_local)
      {
        _lexer.discard_token();
      }
    }

    auto name_expr = parse_primary_expression(_lexer);
    if (name_expr == nullptr) {
      return nullptr;
    } else if (name_expr->get_syntax_type() != syntax_type::identifier) {
      std::cerr << "[parser] Missing function name from 'function' expression." << std::endl;
      return nullptr;
    }

    auto name = expression_cast<identifier>(name_expr)->get_symbol();
    if (_lexer.discard_token().type != token_type::open_paren) {
      std::cerr << "[parser] Expected '(' after name in declaration of function '"
                << name << "'." << std::endl;
      return nullptr;
    }

    std::vector<std::string> parameter_list;
    while (true)
    {
      if (_lexer.token_at().type == token_type::close_paren) {
        _lexer.discard_token();
        break;
      } else if (parameter_list.empty() == false) {
        if (_lexer.token_at().type == token_type::comma) {
          _lexer.discard_token();
        } else {
          std::cerr << "[parser] Expected ',' before parameter #" << (parameter_list.size() + 1)
                    << " in declaration of function '" << name << "'." << std::endl;
          return nullptr;
        }
      }

      auto param_expr = parse_primary_expression(_lexer);
      if (param_expr == nullptr) {
        std::cerr << "[parser] In parameter #" << (parameter_list.size() + 1)
                  << " in declaration of function '" << name << "'." << std::endl;
        return nullptr;
      } else if (param_expr->get_syntax_type() != syntax_type::identifier) {
        std::cerr << "[parser] Expected identifier for parameter #" << (parameter_list.size() + 1)
                  << " in declaration of function '" << name << "'." << std::endl;
        return nullptr;
      }

      parameter_list.push_back(expression_cast<identifier>(param_expr)->get_symbol());
    }

    if (_lexer.discard_token().type != token_type::open_brace) {
      std::cerr << "[parser] Expected '{' after parameter list in declaration of function '"
                << name << "'." << std::endl;
      return nullptr;
    }

    statement::body body;
    while (true)
    {
      if (_lexer.token_at().type == token_type::close_brace) {
        _lexer.discard_token();
        break;
      }

      auto statement = parse_statement(_lexer);
      if (statement == nullptr) {
        std::cerr << "[parser] In body of function '" << name << "'." << std::endl;
        return nullptr;
      }

      body.push_back(statement);
    }

    return expression::make<function_expression>(name, parameter_list, body, global);
  }

  expression::ptr parser::parse_bitwise_expression (lexer& _lexer)
  {
    expression::ptr left = parse_additive_expression(_lexer);
    if (left == nullptr) { return nullptr; }

    while (
      _lexer.token_at().type == token_type::ampersand ||
      _lexer.token_at().type == token_type::pipe ||
      _lexer.token_at().type == token_type::carat
    ) {
      std::string oper = _lexer.discard_token().contents;
      expression::ptr right = parse_additive_expression(_lexer);
      if (right == nullptr) { return nullptr; }

      left = expression::make<binary_expression>(left, right, oper);
    }

    return left;
  }

  expression::ptr parser::parse_additive_expression (lexer& _lexer)
  {
    expression::ptr left = parse_multiplicitive_expression(_lexer);
    if (left == nullptr) { return nullptr; }

    while (
      _lexer.token_at().type == token_type::plus ||
      _lexer.token_at().type == token_type::minus
    ) {
      std::string oper = _lexer.discard_token().contents;
      expression::ptr right = parse_multiplicitive_expression(_lexer);
      if (right == nullptr) { return nullptr; }

      left = expression::make<binary_expression>(left, right, oper);
    }

    return left;
  }

  expression::ptr parser::parse_multiplicitive_expression (lexer& _lexer)
  {
    expression::ptr left = parse_call_expression(_lexer);
    if (left == nullptr) { return nullptr; }

    while (
      _lexer.token_at().type == token_type::asterisk ||
      _lexer.token_at().type == token_type::slash ||
      _lexer.token_at().type == token_type::percent
    ) {
      std::string oper = _lexer.discard_token().contents;
      expression::ptr right = parse_call_expression(_lexer);
      if (right == nullptr) { return nullptr; }

      left = expression::make<binary_expression>(left, right, oper);
    }

    return left;
  }

  expression::ptr parser::parse_call_expression (lexer& _lexer)
  {
    expression::ptr callee = parse_primary_expression(_lexer);
    if (callee == nullptr) { return nullptr; }

    if (
      callee->get_syntax_type() == syntax_type::identifier &&
      _lexer.token_at().type == token_type::open_paren
    )
    {
      _lexer.discard_token();

      auto callee_expr = expression_cast<identifier>(callee);
      expression::array argument_list;
      while (true)
      {
        if (_lexer.token_at().type == token_type::close_paren)
        {
          _lexer.discard_token();
          break;
        }
        else if (argument_list.empty() == false)
        {
          if (_lexer.token_at().type != token_type::comma)
          {
            std::cerr <<  "[parser] Expected ')' after argument list in call to function '"
                      <<  callee_expr->get_symbol() << "'." << std::endl;
            return nullptr;
          }
          _lexer.discard_token();
        }

        expression::ptr expr = parse_expression(_lexer);
        if (expr == nullptr)
        {
          std::cerr <<  "[parser] In argument #" << (argument_list.size() + 1) 
                    << " of call to function '" << callee_expr->get_symbol() << "'."
                    <<  std::endl;
          return nullptr;
        }

        argument_list.push_back(expr);
      }

      return expression::make<call_expression>(callee, argument_list);
    }
    else
    {
      return callee;
    }
  }

  /** Primary Expression Parsing Methods **********************************************************/

  expression::ptr parser::parse_primary_expression (lexer& _lexer, char unary)
  {
    auto token = _lexer.discard_token();
    
    switch (token.type)
    {
      case token_type::minus:
        return parse_primary_expression(_lexer, '-');
      case token_type::identifier:
        return expression::make<identifier>(token.contents);
      case token_type::number:
        switch (unary)
        {
          case '-': return expression::make<numeric_literal>(-token.get_number());
          case '+':
          default: return expression::make<numeric_literal>(token.get_number());
        }
      case token_type::hexadecimal:
      case token_type::integer:
      case token_type::octal:
      case token_type::binary: {
        switch (unary)
        {
          case '-': return expression::make<numeric_literal>(-token.get_number());
          case '+': return expression::make<numeric_literal>(-token.get_number());
          default: return expression::make<numeric_literal>(token.get_integer());
        }
      } break;
      case token_type::string:
        return expression::make<string_literal>(token.contents);
      case token_type::open_paren: {
        expression::ptr expr = parse_expression(_lexer);
        if (expr == nullptr) { return nullptr; }

        if (_lexer.discard_token().type != token_type::close_paren) {
          std::cerr << "[parser] Expected ')' at end of parenthesis-enclosed expression."
                    << std::endl;
          return nullptr;
        }

        return expr;
      } break;
      case token_type::open_bracket: {
        expression::ptr expr = parse_expression(_lexer);
        if (expr == nullptr) { return nullptr; }

        if (_lexer.discard_token().type != token_type::close_bracket) {
          std::cerr << "[parser] Expected ']' at end of address literal expression."
                    << std::endl;
          return nullptr;
        }

        return expression::make<address_literal>(expr);
      } break;
      case token_type::backtick: {
        auto next = _lexer.discard_token();
        if (next.type != token_type::integer)
        {
          std::cerr << "[parser] Expected integer after backtick in pixel literal expression."
                    << std::endl;
          return nullptr;
        }

        const auto& str = next.contents;
        if (str.length() != 8)
        {
          std::cerr << "[parser] Contents of pixel literal must be exactly eight characters long."
                    << std::endl;
          return nullptr;
        }

        std::uint8_t high_byte = 0x00,
                     low_byte = 0x00,
                     high_bit = 0x00,
                     low_bit = 0x00;

        for (std::uint8_t char_index = 0; char_index < 8; ++char_index) {
          std::uint8_t bit = 7 - char_index;

          switch (str.at(char_index)) {
            case '0': high_bit = 0x00; low_bit = 0x00; break;
            case '1': high_bit = 0x00; low_bit = 0x01; break;
            case '2': high_bit = 0x01; low_bit = 0x00; break;
            case '3': high_bit = 0x01; low_bit = 0x01; break;
            default:
              std::cerr << "[parser] Invalid character '" << str.at(char_index)
                        << "' found in pixel literal expression." << std::endl;
              return nullptr;
          }

          low_byte  |= (low_bit  << bit);
          high_byte |= (high_bit << bit);
        }

        return expression::make<numeric_literal>((high_byte << 8) | low_byte);
      } break;
      default:
        std::cerr <<  "[parser] Unexpected '" << token.get_string_type() << "' token = '"
                  <<  token.contents << "'."
                  <<  std::endl;
        return nullptr;
    }
  }

}
