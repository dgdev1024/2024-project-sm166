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
        case language_type::lt_include: return parse_include_statement(_lexer);
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
                << label_identifier->get_symbol() << "'."
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
    return parse_additive_expression(_lexer);
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
    expression::ptr left = parse_primary_expression(_lexer);
    if (left == nullptr) { return nullptr; }

    while (
      _lexer.token_at().type == token_type::asterisk ||
      _lexer.token_at().type == token_type::slash ||
      _lexer.token_at().type == token_type::percent
    ) {
      std::string oper = _lexer.discard_token().contents;
      expression::ptr right = parse_primary_expression(_lexer);
      if (right == nullptr) { return nullptr; }

      left = expression::make<binary_expression>(left, right, oper);
    }

    return left;
  }

  /** Primary Expression Parsing Methods **********************************************************/

  expression::ptr parser::parse_primary_expression (lexer& _lexer)
  {
    auto token = _lexer.discard_token();
    switch (token.type)
    {
      case token_type::identifier:
        return expression::make<identifier>(token.contents);
      case token_type::number:
        return expression::make<numeric_literal>(token.get_number());
      case token_type::hexadecimal:
      case token_type::integer:
      case token_type::octal:
      case token_type::binary:
        return expression::make<numeric_literal>(token.get_integer());
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
      default:
        std::cerr <<  "[parser] Unexpected '" << token.get_string_type() << "' token = '"
                  <<  token.contents << "'."
                  <<  std::endl;
        return nullptr;
    }
  }

}
