/** @file smasm/interpreter.hpp */

#pragma once

#include <smasm/values.hpp>
#include <smasm/lexer.hpp>
#include <smasm/parser.hpp>
#include <smasm/assembly.hpp>
#include <smasm/environment.hpp>

namespace smasm
{

  class interpreter
  {
  public:
    interpreter (
      lexer& _lexer,
      parser& _parser,
      assembly& _assembly,
      environment& _environment
    );

  public:
    value::ptr evaluate (const statement::ptr& stmt);

  private:
    value::ptr evaluate (const statement::ptr& stmt,
      environment& env);
    value::ptr evaluate_program (const program* prgm,
      environment& env);
    value::ptr evaluate_size_directive (const size_directive* dir,
      environment& env);
    value::ptr evaluate_section_directive (const section_directive* dir,
      environment& env);
    value::ptr evaluate_variable_declaration_statement (const variable_declaration_statement* stmt,
      environment& env);
    value::ptr evaluate_label_statement (const label_statement* stmt,
      environment& env);
    value::ptr evaluate_data_statement (const data_statement* stmt,
      environment& env);
    value::ptr evaluate_include_statement (const include_statement* stmt,
      environment& env);
    value::ptr evaluate_instruction_statement (const instruction_statement* stmt,
      environment& env);
    value::ptr evaluate_identifier (const identifier* expr,
      environment& env);
    value::ptr evaluate_function_expression (const function_expression* expr,
      environment& env);
    value::ptr evaluate_call_expression (const call_expression* expr,
      environment& env);
    value::ptr evaluate_address_expression (const address_literal* expr,
      environment& env);

  private:
    bool evaluate_inst_ld (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_lh (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_st (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_sh (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_mv (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_ms (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_push (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_pop (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_jmp (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_call (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_rst (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_ret (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_inc (const instruction_statement* stmt, environment& env);
    bool evaluate_inst_dec (const instruction_statement* stmt, environment& env);
  
  private:
    bool evaluate_inst_gen_a (const instruction_statement* stmt, std::uint16_t _opcode,
      environment& env);
    bool evaluate_inst_gen_b (const instruction_statement* stmt, std::uint16_t _opcode,
      environment& env);
    bool evaluate_inst_gen_c (const instruction_statement* stmt, std::uint16_t _opcode,
      environment& env);

  private:
    lexer& m_lexer;
    parser& m_parser;
    assembly& m_assembly;
    environment& m_environment;

  };
  
}
