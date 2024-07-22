/** @file smasm/interpreter.cpp */

#include <smasm/interpreter.hpp>

namespace smasm
{

  interpreter::interpreter (
    lexer& _lexer,
    parser& _parser,
    assembly& _assembly,
    environment& _environment
  ) :
    m_lexer { _lexer },
    m_parser { _parser },
    m_assembly { _assembly },
    m_environment { _environment }
  {
    
  }

  value::ptr interpreter::evaluate (const statement::ptr& stmt)
  {
    return evaluate(stmt, m_environment);
  }

  value::ptr interpreter::evaluate (const statement::ptr& stmt, environment& env)
  {
    switch (stmt->get_syntax_type())
    {
      case syntax_type::program:
        return evaluate_program(
          statement_cast<program>(stmt).get(), env
        );
      case syntax_type::size_directive:
        return evaluate_size_directive(
          statement_cast<size_directive>(stmt).get(), env
        );
      case syntax_type::section_directive:
        return evaluate_section_directive(
          statement_cast<section_directive>(stmt).get(), env
        );
      case syntax_type::variable_declaration_statement:
        return evaluate_variable_declaration_statement(
          statement_cast<variable_declaration_statement>(stmt).get(), env
        );
      case syntax_type::label_statement:
        return evaluate_label_statement(
          statement_cast<label_statement>(stmt).get(), env
        );
      case syntax_type::data_statement:
        return evaluate_data_statement(
          statement_cast<data_statement>(stmt).get(), env
        );
      case syntax_type::include_statement:
        return evaluate_include_statement(
          statement_cast<include_statement>(stmt).get(), env
        );
      case syntax_type::instruction_statement:
        return evaluate_instruction_statement(
          statement_cast<instruction_statement>(stmt).get(), env
        );
      case syntax_type::function_expression:
        return evaluate_function_expression(
          statement_cast<function_expression>(stmt).get(), env
        );
      case syntax_type::call_expression:
        return evaluate_call_expression(
          statement_cast<call_expression>(stmt).get(), env
        );
      case syntax_type::identifier:
        return evaluate_identifier(
          statement_cast<identifier>(stmt).get(), env
        );
      case syntax_type::address_literal:
        return evaluate_address_expression(
          statement_cast<address_literal>(stmt).get(), env
        );
      case syntax_type::numeric_literal:
        return value::make<number_value>(
          statement_cast<numeric_literal>(stmt)->get_number()
        );
      case syntax_type::string_literal:
        return value::make<string_value>(
          statement_cast<string_literal>(stmt)->get_string()
        );
      default:
        std::cerr << "[interpreter] Un-implemented syntax node encountered."
                  << std::endl;
        stmt->dump(std::cerr, 2);
        return nullptr;
    }
  }

  value::ptr interpreter::evaluate_program (const program* prgm,
    environment& env)
  {
    value::ptr last_evaluated = value::make<void_value>();
    
    if (prgm == nullptr) {
      return nullptr;
    }

    for (const auto& stmt : prgm->get_body())
    {
      last_evaluated = evaluate(stmt, env);
      if (last_evaluated == nullptr) { return nullptr; }
    }

    return last_evaluated;
  }

  value::ptr interpreter::evaluate_size_directive (const size_directive* dir,
    environment& env)
  {
    (void) env;

    if (m_assembly.resize_rom(dir->get_size()) == false) {
      return nullptr;
    }
    
    return value::make<void_value>();
  }

  value::ptr interpreter::evaluate_section_directive (const section_directive* dir,
    environment& env)
  {
    value::ptr cursor_value = evaluate(dir->get_address_expr(), env);
    if (cursor_value->get_value_type() != value_type::number) {
      std::cerr <<  "[interpreter] Expected numeric value in second parameter of '.section' directive."
                <<  std::endl;
      return nullptr;
    }

    auto casted_value = value_cast<number_value>(cursor_value);
    if (dir->is_ram() == true) {
      m_assembly.set_ram_mode(true);
      m_assembly.set_ram_cursor(casted_value->get_integer());
    } else {
      m_assembly.set_ram_mode(false);
      if (m_assembly.set_rom_cursor(casted_value->get_integer()) == false) {
        return nullptr;
      }
    }

    return value::make<void_value>();
  }

  value::ptr interpreter::evaluate_variable_declaration_statement
    (const variable_declaration_statement* stmt, environment& env)
  {
    auto key_expr = expression_cast<identifier>(stmt->get_key_expr());
    auto value = evaluate(stmt->get_value_expr(), env);
    if (value == nullptr) {
      std::cerr <<  "[interpreter] Could not evaluate value in declaration of variable '"
                <<  key_expr->get_symbol() << "." << std::endl;
      return nullptr;
    } else if (value->get_value_type() == value_type::none) {
      std::cerr <<  "[interpreter] Declaration of variable '" << key_expr->get_symbol()
                <<  "' has evaluated to no value." << std::endl;
      return nullptr;
    }

    if (
      env.declare_variable(
        key_expr->get_symbol(),
        value,
        stmt->is_constant()
      ) == false
    ) {
      return nullptr;
    }

    return value::make<void_value>();
  }

  value::ptr interpreter::evaluate_label_statement (const label_statement* stmt,
    environment& env)
  {
    auto label_expr = expression_cast<identifier>(stmt->get_label());
    if (
      env.declare_variable(
        label_expr->get_symbol(), 
        value::make<address_value>(m_assembly.get_current_cursor())
      ) == false
    ) {
      return nullptr;
    }

    return value::make<void_value>();
  }

  value::ptr interpreter::evaluate_data_statement (const data_statement* stmt,
    environment& env)
  {
    const auto& exprs = stmt->get_array();

    if (m_assembly.is_in_ram() == true) {
      for (std::size_t i = 0; i < exprs.size(); ++i) {
        value::ptr val = evaluate(exprs.at(i), env);
        if (val == nullptr) { return nullptr; }

        if (val->get_value_type() != value_type::number) {
          std::cerr << "[interpreter] Expression #" << i + 1 << " in RAM data statement is "
                    << "not numeric." << std::endl;
          return nullptr;
        }

        auto count = value_cast<number_value>(val)->get_integer();
        auto cursor = m_assembly.get_ram_cursor();
        m_assembly.set_ram_cursor(cursor + (count * stmt->get_size()));
      }
    } else {
      for (std::size_t i = 0; i < exprs.size(); ++i) {
        value::ptr val = evaluate(exprs.at(i), env);
        if (val == nullptr) { return nullptr; }
        
        if (val->get_value_type() == value_type::number) {
          auto integer = value_cast<number_value>(val)->get_integer();
          auto size = stmt->get_size();
          bool ok = true;

          if (size == 4)           { ok = m_assembly.write_long(integer); }
          else if (size == 2)      { ok = m_assembly.write_word(integer & 0xFFFF); }
          else if (size == 1)      { ok = m_assembly.write_byte(integer & 0xFF); }
          else                     { ok = false; }

          if (ok == false) { return nullptr; }
        } else if (val->get_value_type() == value_type::string) {
          if (stmt->get_size() != 1) {
            std::cerr << "[interpreter] Strings can only be passed into byte data statements."
                      << std::endl;
            return nullptr;
          }

          auto str = value_cast<string_value>(val)->get_string();

          for (const char c : str) {
            if (m_assembly.write_byte((std::uint8_t) c) == false) {
              return nullptr;
            }
          }

          if (m_assembly.write_byte(0x00) == false) { return nullptr; }
        }
      }
    }

    return value::make<void_value>();
  }
  
  value::ptr interpreter::evaluate_include_statement (const include_statement* stmt,
    environment& env)
  {
    auto string_expr = expression_cast<string_literal>(stmt->get_filename_expr());
    if (m_lexer.lex_file(m_lexer.get_parent_path() / string_expr->get_string()) == false)
    {
      std::cerr << "[interpreter] Could not lex included source file \""
                << string_expr->get_string() << "\"." << std::endl;
      return nullptr;
    }

    auto program = m_parser.parse_program(m_lexer);
    return evaluate_program(program.get(), env);
  }

  value::ptr interpreter::evaluate_instruction_statement (const instruction_statement* stmt,
    environment& env)
  {
    bool ok = true;
    
    switch (keyword::lookup(stmt->get_mnemonic()).param_one)
    {
      case instruction_type::it_nop:  ok = m_assembly.write_word(0x0000); break;
      case instruction_type::it_stop: ok = m_assembly.write_word(0x0001); break;
      case instruction_type::it_halt: ok = m_assembly.write_word(0x0002); break;
      case instruction_type::it_di:   ok = m_assembly.write_word(0x0003); break;
      case instruction_type::it_ei:   ok = m_assembly.write_word(0x0004); break;
      case instruction_type::it_daa:  ok = m_assembly.write_word(0x0005); break;
      case instruction_type::it_cpl:  ok = m_assembly.write_word(0x0006); break;
      case instruction_type::it_ccf:  ok = m_assembly.write_word(0x0007); break;
      case instruction_type::it_scf:  ok = m_assembly.write_word(0x0008); break;
      case instruction_type::it_ld:   ok = evaluate_inst_ld(stmt, env); break;
      case instruction_type::it_lhb:
      case instruction_type::it_lhr:
      case instruction_type::it_lhw:  ok = evaluate_inst_lh(stmt, env); break;
      case instruction_type::it_st:   ok = evaluate_inst_st(stmt, env); break;
      case instruction_type::it_shb:
      case instruction_type::it_shr:
      case instruction_type::it_shw:
      case instruction_type::it_ssp:
      case instruction_type::it_spc:  ok = evaluate_inst_sh(stmt, env); break;
      case instruction_type::it_mv:   ok = evaluate_inst_mv(stmt, env); break;
      case instruction_type::it_msp:  
      case instruction_type::it_mpc:  ok = evaluate_inst_ms(stmt, env); break;
      case instruction_type::it_push: ok = evaluate_inst_push(stmt, env); break;
      case instruction_type::it_pop:  ok = evaluate_inst_pop(stmt, env); break;
      case instruction_type::it_jmp:  ok = evaluate_inst_jmp(stmt, env); break;
      case instruction_type::it_call: ok = evaluate_inst_call(stmt, env); break;
      case instruction_type::it_rst:  ok = evaluate_inst_rst(stmt, env); break;
      case instruction_type::it_ret:  ok = evaluate_inst_ret(stmt, env); break;
      case instruction_type::it_reti: ok = m_assembly.write_word(0x2310); break;
      case instruction_type::it_inc:  ok = evaluate_inst_inc(stmt, env); break;
      case instruction_type::it_dec:  ok = evaluate_inst_dec(stmt, env); break;
      case instruction_type::it_add:  ok = evaluate_inst_gen_a(stmt, 0x3200, env); break;
      case instruction_type::it_adc:  ok = evaluate_inst_gen_a(stmt, 0x3240, env); break;
      case instruction_type::it_sub:  ok = evaluate_inst_gen_a(stmt, 0x3300, env); break;
      case instruction_type::it_sbc:  ok = evaluate_inst_gen_a(stmt, 0x3340, env); break;
      case instruction_type::it_and:  ok = evaluate_inst_gen_a(stmt, 0x5000, env); break;
      case instruction_type::it_or:   ok = evaluate_inst_gen_a(stmt, 0x5100, env); break;
      case instruction_type::it_xor:  ok = evaluate_inst_gen_a(stmt, 0x5200, env); break;
      case instruction_type::it_cmp:  ok = evaluate_inst_gen_a(stmt, 0x5300, env); break;
      case instruction_type::it_bit:  ok = evaluate_inst_gen_c(stmt, 0x6000, env); break;
      case instruction_type::it_set:  ok = evaluate_inst_gen_c(stmt, 0x6100, env); break;
      case instruction_type::it_res:  ok = evaluate_inst_gen_c(stmt, 0x6200, env); break;
      case instruction_type::it_sla:  ok = evaluate_inst_gen_b(stmt, 0x7000, env); break;
      case instruction_type::it_sra:  ok = evaluate_inst_gen_b(stmt, 0x7100, env); break;
      case instruction_type::it_srl:  ok = evaluate_inst_gen_b(stmt, 0x7200, env); break;
      case instruction_type::it_rl:   ok = evaluate_inst_gen_b(stmt, 0x7300, env); break;
      case instruction_type::it_rla:  ok = m_assembly.write_word(0x7340); break;
      case instruction_type::it_rlc:  ok = evaluate_inst_gen_b(stmt, 0x7400, env); break;
      case instruction_type::it_rlca: ok = m_assembly.write_word(0x7440); break;
      case instruction_type::it_rr:   ok = evaluate_inst_gen_b(stmt, 0x7500, env); break;
      case instruction_type::it_rra:  ok = m_assembly.write_word(0x7540); break;
      case instruction_type::it_rrc:  ok = evaluate_inst_gen_b(stmt, 0x7600, env); break;
      case instruction_type::it_rrca: ok = m_assembly.write_word(0x7640); break;

      default:
        std::cerr << "[interpreter] Un-implemented instruction mnemonic: '"
                  << stmt->get_mnemonic() << "'." << std::endl;
        return nullptr;
    }

    return (ok == true) ? value::make<void_value>() : nullptr;
  }

  /** Expression Evaluation Methods ***************************************************************/

  value::ptr interpreter::evaluate_identifier (const identifier* expr,
    environment& env)
  {
    const auto& keyword = expr->get_keyword();
    switch (keyword.type)
    {
      case keyword_type::cpu_register:
        return value::make<cpu_register_value>((register_type) keyword.param_one);
      case keyword_type::condition:
        return value::make<cpu_condition_value>((condition_type) keyword.param_one);
      default: {
        const auto& variable = env.resolve_variable(expr->get_symbol());
        if (variable == nullptr) {
          return nullptr;
        }

        return variable;
      } break;
    }

    return nullptr;
  }

  value::ptr interpreter::evaluate_function_expression (const function_expression* expr,
    environment& env)
  {
    auto value = value::make<function_value>(expr->get_name(), expr->get_parameter_list(),
      expr->get_body());

    if (
      env.declare_variable(
        expr->get_name(),
        value,
        true
      ) == false
    ) {
      return nullptr;
    }

    return value;
  }

  value::ptr interpreter::evaluate_call_expression (const call_expression* expr,
    environment& env)
  {
    auto callee_expr = expression_cast<identifier>(expr->get_callee_expr());
    const auto& variable = env.resolve_variable(callee_expr->get_symbol());
    if (variable == nullptr)
    {
      return nullptr;
    }
    else if (variable->get_value_type() != value_type::function)
    {
      std::cerr <<  "[interpreter] Identifier '" << callee_expr->get_symbol()
                <<  "' does not resolve to a function." << std::endl;
      return nullptr;
    }

    auto function_val = value_cast<function_value>(variable);
    environment scope_env { &env };
    const auto& argument_list = expr->get_argument_list();
    const auto& parameter_list = function_val->get_parameter_list();
    for (std::size_t i = 0; i < argument_list.size(); ++i)
    {
      value::ptr evaluated_value = evaluate(argument_list.at(i), env);
      if (evaluated_value == nullptr)
      {
        std::cerr <<  "[interpreter] Evaluating argument #" << (i + 1)
                  <<  " of call to function '" << callee_expr->get_symbol() << "'."
                  <<  std::endl;
        return nullptr;
      }

      scope_env.declare_variable("_" + std::to_string(i), evaluated_value);
      if (i < parameter_list.size())
      {
        scope_env.declare_variable(parameter_list.at(i), evaluated_value);
      }
    }

    value::ptr last_evaluated = value::make<void_value>();
    for (const auto& stmt : function_val->get_body())
    {
      last_evaluated = evaluate(stmt, scope_env);
      if (last_evaluated == nullptr) 
      { 
        std::cerr <<  "[interpreter] Evaluating call to function '" 
                  <<  callee_expr->get_symbol() << "'."
                  <<  std::endl;
        return nullptr; 
      }
    }
    
    return last_evaluated;
  }

  value::ptr interpreter::evaluate_address_expression (const address_literal* expr,
    environment& env)
  {
    auto addr_value = evaluate(expr->get_address_expr(), env);
    if (addr_value->get_value_type() == value_type::number) {
      return value::make<address_value>(
        value_cast<number_value>(addr_value)->get_integer()
      );
    } else if (addr_value->get_value_type() == value_type::cpu_register) {
      return value::make<cpu_register_value>(
        value_cast<cpu_register_value>(addr_value)->get_type(), true
      );
    } else {
      std::cerr << "[interpreter] Expected number value or long register in address expression." 
                << std::endl;
      return nullptr;
    }
  }

}
