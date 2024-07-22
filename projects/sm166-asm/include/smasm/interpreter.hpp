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
    value::ptr evaluate_program (const program* prgm);
    value::ptr evaluate_size_directive (const size_directive* dir);
    value::ptr evaluate_section_directive (const section_directive* dir);
    value::ptr evaluate_label_statement (const label_statement* stmt);
    value::ptr evaluate_data_statement (const data_statement* stmt);
    value::ptr evaluate_include_statement (const include_statement* stmt);
    value::ptr evaluate_instruction_statement (const instruction_statement* stmt);
    value::ptr evaluate_identifier (const identifier* expr);
    value::ptr evaluate_address_expression (const address_literal* expr);

  private:
    bool evaluate_inst_ld (const instruction_statement* stmt);
    bool evaluate_inst_lh (const instruction_statement* stmt);
    bool evaluate_inst_st (const instruction_statement* stmt);
    bool evaluate_inst_sh (const instruction_statement* stmt);
    bool evaluate_inst_mv (const instruction_statement* stmt);
    bool evaluate_inst_ms (const instruction_statement* stmt);
    bool evaluate_inst_push (const instruction_statement* stmt);
    bool evaluate_inst_pop (const instruction_statement* stmt);
    bool evaluate_inst_jmp (const instruction_statement* stmt);
    bool evaluate_inst_call (const instruction_statement* stmt);
    bool evaluate_inst_rst (const instruction_statement* stmt);
    bool evaluate_inst_ret (const instruction_statement* stmt);
    bool evaluate_inst_inc (const instruction_statement* stmt);
    bool evaluate_inst_dec (const instruction_statement* stmt);
  
  private:
    bool evaluate_inst_gen_a (const instruction_statement* stmt, std::uint16_t _opcode);
    bool evaluate_inst_gen_b (const instruction_statement* stmt, std::uint16_t _opcode);
    bool evaluate_inst_gen_c (const instruction_statement* stmt, std::uint16_t _opcode);

  private:
    lexer& m_lexer;
    parser& m_parser;
    assembly& m_assembly;
    environment& m_environment;

  };
  
}
