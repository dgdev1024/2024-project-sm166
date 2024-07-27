/** @file smasm/parser.hpp */

#pragma once

#include <smasm/lexer.hpp>
#include <smasm/syntax.hpp>

namespace smasm
{

  class parser
  {
  public:
    program::ptr    parse_program (lexer& _lexer);

  private:
    statement::ptr  parse_directive (lexer& _lexer);
    statement::ptr  parse_size_directive (lexer& _lexer);
    statement::ptr  parse_section_directive (lexer& _lexer);

  private:
    statement::ptr  parse_statement (lexer& _lexer);
    statement::ptr  parse_variable_declaration_statement (lexer& _lexer, bool constant);
    statement::ptr  parse_label_statement (lexer& _lexer);
    statement::ptr  parse_data_statement (lexer& _lexer, int size);
    statement::ptr  parse_repeat_statement (lexer& _lexer);
    statement::ptr  parse_shift_statement (lexer& _lexer);
    statement::ptr  parse_include_statement (lexer& _lexer);
    statement::ptr  parse_incbin_statement (lexer& _lexer);
    statement::ptr  parse_instruction_statement (lexer& _lexer);
  
  private:
    expression::ptr parse_expression (lexer& _lexer);
    expression::ptr parse_function_expression (lexer& _lexer);
    expression::ptr parse_bitwise_expression (lexer& _lexer);
    expression::ptr parse_multiplicitive_expression (lexer& _lexer);
    expression::ptr parse_additive_expression (lexer& _lexer);
    expression::ptr parse_call_expression (lexer& _lexer);
  
  private:
    expression::ptr parse_primary_expression (lexer& _lexer, char unary = '\0');

  };

}
