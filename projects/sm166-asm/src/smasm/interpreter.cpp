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
    m_first = true;
    m_lexer.clear_includes();
    m_assembly.clear_incbins();
    if (evaluate(stmt, m_environment) == nullptr)
    {
      return nullptr;
    }

    m_first = false;
    m_lexer.clear_includes();
    m_assembly.clear_incbins();
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
      case syntax_type::repeat_statement:
        return evaluate_repeat_statement(
          statement_cast<repeat_statement>(stmt).get(), env
        );
      case syntax_type::shift_statement:
        return evaluate_shift_statement(
          statement_cast<shift_statement>(stmt).get(), env
        );
      case syntax_type::if_statement:
        return evaluate_if_statement(
          statement_cast<if_statement>(stmt).get(), env
        );
      case syntax_type::include_statement:
        return evaluate_include_statement(
          statement_cast<include_statement>(stmt).get(), env
        );
      case syntax_type::incbin_statement:
        return evaluate_incbin_statement(
          statement_cast<incbin_statement>(stmt).get(), env
        );
      case syntax_type::instruction_statement:
        return evaluate_instruction_statement(
          statement_cast<instruction_statement>(stmt).get(), env
        );
      case syntax_type::charmap_statement:
        return evaluate_charmap_statement(
          statement_cast<charmap_statement>(stmt).get(), env
        );
      case syntax_type::newcharmap_statement:
        return evaluate_newcharmap_statement(
          statement_cast<newcharmap_statement>(stmt).get(), env
        );
      case syntax_type::setcharmap_statement:
        return evaluate_setcharmap_statement(
          statement_cast<setcharmap_statement>(stmt).get(), env
        );
      case syntax_type::function_expression:
        return evaluate_function_expression(
          statement_cast<function_expression>(stmt).get(), env
        );
      case syntax_type::call_expression:
        return evaluate_call_expression(
          statement_cast<call_expression>(stmt).get(), env
        );
      case syntax_type::unary_expression:
        return evaluate_unary_expression(
          statement_cast<unary_expression>(stmt).get(), env
        );
      case syntax_type::binary_expression:
        return evaluate_binary_expression(
          statement_cast<binary_expression>(stmt).get(), env
        );
      case syntax_type::identifier:
        return evaluate_identifier(
          statement_cast<identifier>(stmt).get(), env
        );
      case syntax_type::address_literal:
        return evaluate_address_expression(
          statement_cast<address_literal>(stmt).get(), env
        );
      case syntax_type::numeric_literal: {
        auto nl = statement_cast<numeric_literal>(stmt);

        return value::make<number_value>(
          nl->get_integer(),
          nl->get_fractional(),
          nl->get_fraction_bits()
        );
      } break;
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
      env.declare_variable("_union", casted_value);
    } else {
      m_assembly.set_ram_mode(false);
      if (m_assembly.set_rom_cursor(casted_value->get_integer()) == false) {
        return nullptr;
      }
      env.declare_variable("_union", value::make<void_value>());
    }

    return value::make<void_value>();
  }

  value::ptr interpreter::evaluate_variable_declaration_statement
    (const variable_declaration_statement* stmt, environment& env)
  {
    // std::string key = expression_cast<identifier>(stmt->get_key_expr())->get_symbol();
    std::string key = "";
    auto key_expr = stmt->get_key_expr();
    if (key_expr->get_syntax_type() == syntax_type::identifier)
    {
      key = expression_cast<identifier>(key_expr)->get_symbol();
      if (key.starts_with('_'))
      {
        value::ptr result = evaluate(key_expr, env);
        if (
          result != nullptr &&
          result->get_value_type() == value_type::string
        )
        {
          key =
            value_cast<string_value>(result)->get_string();
        }
      }
    }
    else if (key_expr->get_syntax_type() == syntax_type::string_literal)
    {
      key = expression_cast<string_literal>(key_expr)->get_string();
    }
    else if (key_expr->get_syntax_type() == syntax_type::binary_expression)
    {
      value::ptr result = evaluate(key_expr, env);
      if (result == nullptr) { return nullptr; }

      if (result->get_value_type() == value_type::string)
      {
        key = value_cast<string_value>(result)->get_string();
      }
      else
      {
        return nullptr;
      }
    }

    auto value = evaluate(stmt->get_value_expr(), env);
    if (value == nullptr) {
      std::cerr <<  "[interpreter] Could not evaluate value in declaration of variable '"
                <<  key << "'." << std::endl;
      return nullptr;
    } else if (value->get_value_type() == value_type::none) {
      std::cerr <<  "[interpreter] Declaration of variable '" << key
                <<  "' has evaluated to no value." << std::endl;
      return nullptr;
    }

    environment& target_env = (
      stmt->is_global() == true ?
        m_environment :
        env
    );
    
    if (
      target_env.declare_variable(
        key,
        value,
        m_first == false && stmt->is_constant()
      ) == false
    ) {
      return nullptr;
    }

    return value::make<void_value>();
  }

  value::ptr interpreter::evaluate_label_statement (const label_statement* stmt,
    environment& env)
  {
    std::string label = "";
    auto label_expr = stmt->get_label();
    if (label_expr->get_syntax_type() == syntax_type::identifier)
    {
      label = expression_cast<identifier>(label_expr)->get_symbol();
    }
    else
    {
      auto label_value = evaluate(label_expr, env);
      if (label_value == nullptr) { return nullptr; }
      else if (label_value->get_value_type() != value_type::string) {
        return nullptr;
      }
      
      label = value_cast<string_value>(label_value)->get_string();
    }
    
    if (keyword::lookup(label).type != keyword_type::none)
    {
      return nullptr;
    }
  
    if (
      m_environment.declare_variable(
        label,
        value::make<number_value>(m_assembly.get_current_cursor())
      ) == false
    ) {
      return nullptr;
    }

    // std::cout << label_expr->get_symbol() << " = " << m_assembly.get_current_cursor() << std::endl;

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
      const auto& charmap = m_assembly.get_current_charmap();
    
      for (std::size_t i = 0; i < exprs.size(); ++i) {
        value::ptr val = evaluate(exprs.at(i), env);
        if (val == nullptr) { return nullptr; }
        
        if (val->get_value_type() == value_type::number) {
          auto integer = value_cast<number_value>(val)->get_integer();
          auto size = stmt->get_size();
          bool ok = true;

          if (size == 4)           { ok = m_assembly.write_long(integer & 0xFFFFFFFF); }
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

          for (std::size_t ci = 0; ci < str.length(); ++ci) {
            const char& c = str.at(ci);
            if (c == '<')
            {
              std::size_t closing_brace = str.find('>', ci);
              if (closing_brace != std::string::npos)
              {
                auto substring = str.substr(ci, closing_brace - ci + 1);
                
                if (charmap.contains(substring) == true)
                {
                  bool result = m_assembly.write_byte(charmap.at(substring));
                  if (result == false) { return nullptr; }
                  else
                  {
                    ci = closing_brace;
                    continue;
                  }
                }
              }
            }
            
            bool result = m_assembly.write_byte(
              (charmap.contains(std::string { c }) == true) ?
                charmap.at(std::string { c }) :
                (std::uint8_t) c
            );
          
            if (result == false) {
              return nullptr;
            }
          }

          if (m_assembly.write_byte(0x00) == false) { return nullptr; }
        }
      }
    }

    return value::make<void_value>();
  }
  
  value::ptr interpreter::evaluate_repeat_statement (const repeat_statement* stmt,
    environment& env)
  {
    value::ptr evaluated_count = evaluate(stmt->get_count_expr(), env);
    if (evaluated_count == nullptr)
    {
      return nullptr;
    }
    else if (evaluated_count->get_value_type() != value_type::number)
    {
      return nullptr;
    }
  
    environment scope_env { &env, environment_scope::repeat };
    value::ptr last_evaluated = value::make<void_value>();
    for (std::uint64_t i = 0; i < value_cast<number_value>(evaluated_count)->get_integer(); ++i)
    {
      scope_env.declare_variable("_iter", value::make<number_value>(i), false);
    
      for (const auto& body_stmt : stmt->get_body())
      {
        last_evaluated = evaluate(body_stmt, *env.get_function_scope(true));
        if (last_evaluated == nullptr)
        {
          std::cerr << "[interpreter] In iteration #" << i << " of repeat statement." << std::endl;
          return nullptr;
        }
      }
    }
  
    return value::make<void_value>();
  }
  
  value::ptr interpreter::evaluate_shift_statement (const shift_statement* stmt,
    environment& env)
  {
    auto number_expr = expression_cast<numeric_literal>(stmt->get_count_expr());
    environment* function_scope = env.get_function_scope();
    if (function_scope == nullptr)
    {
      std::cerr << "[interpreter] The 'shift' statement requires a function scope." << std::endl;
      return nullptr;
    }
    else
    {
      function_scope->shift_arguments(number_expr->get_integer());
    }
  
    return value::make<void_value>();
  }
  
  value::ptr interpreter::evaluate_if_statement (const if_statement* stmt,
    environment& env)
  {
    value::ptr clause_value = evaluate(stmt->get_clause_expr(), env);
    if (clause_value == nullptr) { return nullptr; }
    
    const statement::body* target_body = &stmt->get_then_body();
    if (clause_value->is_truthy() == false)
    {
      target_body = &stmt->get_else_body();
    }
    
    value::ptr last_evaluated = value::make<void_value>();
    for (const auto& body_stmt : *target_body)
    {
      last_evaluated = evaluate(body_stmt, *env.get_function_scope(true));
      if (last_evaluated == nullptr)
      {
        return nullptr;
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
  
  value::ptr interpreter::evaluate_incbin_statement (const incbin_statement* stmt,
    environment& env)
  {
    (void) env;
  
    auto string_expr = expression_cast<string_literal>(stmt->get_filename_expr());
    if (m_assembly.include_binary(m_lexer.get_parent_path() / string_expr->get_string()) == false)
    {
      std::cerr << "[interpreter] Could not write included binary file \""
                << string_expr->get_string() << "\"." << std::endl;
      return nullptr;
    }

    return value::make<void_value>();
  }
  
  value::ptr interpreter::evaluate_charmap_statement (const charmap_statement* stmt,
    environment& env)
  {
    value::ptr char_value = evaluate(stmt->get_char_expr(), env);
    if (char_value == nullptr) { return nullptr; }
    else if (char_value->get_value_type() != value_type::string)
    {
      std::cerr << "[interpreter] "
                << "Expected string for first argument to 'charmap' statement."
                << std::endl;
      return nullptr;
    }
    
    value::ptr map_value = evaluate(stmt->get_map_expr(), env);
    if (map_value == nullptr) { return nullptr; }
    else if (map_value->get_value_type() != value_type::number)
    {
      std::cerr << "[interpreter] "
                << "Expected number for second argument to 'charmap' statement."
                << std::endl;
      return nullptr;
    }
    
    auto& charmap = m_assembly.get_current_charmap();
    auto mapping = value_cast<string_value>(char_value)->get_string();
    
    if (mapping == "")
    {
      std::cerr << "[interpreter] "
                << "String argument to 'charmap' statement cannot be blank."
                << std::endl;
      return nullptr;
    }
    
    if (mapping.starts_with("<") && mapping.ends_with(">"))
    {
      charmap[mapping] = value_cast<number_value>(map_value)->get_integer();
    }
    else
    {
      charmap[mapping.substr(0, 1)] = value_cast<number_value>(map_value)->get_integer();
    }
    
    return value::make<void_value>();
  }
  
  value::ptr interpreter::evaluate_newcharmap_statement (const newcharmap_statement* stmt,
    environment& env)
  {
    value::ptr name_value = evaluate(stmt->get_name_expr(), env);
    if (name_value == nullptr) { return nullptr; }
    else if (name_value->get_value_type() != value_type::string)
    {
      std::cerr << "[interpreter] "
                << "Expected string for argument to 'newcharmap' statement."
                << std::endl;
      return nullptr;
    }
    
    const auto& name = value_cast<string_value>(name_value)->get_string();
    m_assembly.set_current_charmap(name, true);
    
    return value::make<void_value>();
  }
  
  value::ptr interpreter::evaluate_setcharmap_statement (const setcharmap_statement* stmt,
    environment& env)
  {
    value::ptr name_value = evaluate(stmt->get_name_expr(), env);
    if (name_value == nullptr) { return nullptr; }
    else if (name_value->get_value_type() != value_type::string)
    {
      std::cerr << "[interpreter] "
                << "Expected string for argument to 'setcharmap' statement."
                << std::endl;
      return nullptr;
    }
    
    const auto& name = value_cast<string_value>(name_value)->get_string();
    if (m_assembly.set_current_charmap(name) == false)
    {
      std::cerr << "[interpreter] "
                << "Charmap '" << name << "' not found."
                << std::endl;
      return nullptr;
    }
    
    return value::make<void_value>();
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
        const auto& variable = env.resolve_variable(expr->get_symbol(), m_first);
        if (variable == nullptr) {
          if (m_first == true) {
            return value::make<number_value>(0);
          }

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

    environment& target_env = (
      expr->is_global() == true ?
        m_environment :
        env
    );

    if (
      target_env.declare_variable(
        expr->get_name(),
        value,
        m_first == false
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
    else if (
      variable->get_value_type() != value_type::function
    )
    {
      std::cerr <<  "[interpreter] Identifier '" << callee_expr->get_symbol()
                <<  "' does not resolve to a function." << std::endl;
      return nullptr;
    }

    auto function_val = value_cast<function_value>(variable);
    environment scope_env { &env, environment_scope::function };
    const auto& argument_list = expr->get_argument_list();
    const auto& parameter_list = function_val->get_parameter_list();
    scope_env.declare_variable("_count", value::make<number_value>(argument_list.size()));
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

    if (function_val->is_native() == true)
    {
      return function_val->get_native()(scope_env);
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

  namespace priv
  {

    value::ptr unary_number (const value::ptr& val, const std::string& oper)
    {
      auto cast = value_cast<number_value>(val);

      if (oper == "+")      
      { 
        return value::make<number_value>(+cast->get_integer(),
          cast->get_fractional(), cast->get_fraction_bits()); 
      }
      else if (oper == "-") 
      { 
        return value::make<number_value>(-cast->get_integer(),
          cast->get_fractional(), cast->get_fraction_bits()); 
      }
      else if (oper == "~") 
      { 
        return value::make<number_value>(~cast->get_integer()); 
      }
      
      std::cerr << "[interpreter] Invalid operation '" << oper
                << "' encountered in unary number expression."
                << std::endl;
      return nullptr;
    }

    value::ptr unary_address (const value::ptr& val, const std::string& oper)
    {
      auto cast = value_cast<address_value>(val);
      if (oper == "~") { return value::make<address_value>(~cast->get_address()); }
      
      std::cerr << "[interpreter] Invalid operation '" << oper
                << "' encountered in unary address expression."
                << std::endl;
      return nullptr;
    }

  }

  value::ptr interpreter::evaluate_unary_expression (const unary_expression* expr,
    environment& env)
  {
    auto eval = evaluate(expr->get_expr(), env);
    if (eval == nullptr) { return nullptr; }

    switch (eval->get_value_type())
    {
      // case value_type::number:  return priv::unary_number(eval, expr->get_oper());
      case value_type::number: {
        auto test = priv::unary_number(eval, expr->get_oper());
        // std::cout << std::hex << value_cast<number_value>(test)->get_integer() << std::endl;
        // std::cout << std::dec;
        return test;
      } break;
      case value_type::address: return priv::unary_address(eval, expr->get_oper());
      default:                  return nullptr;
    }
  }
  
  namespace priv
  {
  
    value::ptr number_vs_number (const value::ptr& lhs, const value::ptr& rhs,
      const std::string& oper)
    {
      auto left  = value_cast<number_value>(lhs);
      auto right = value_cast<number_value>(rhs);
      
      if (oper == "+")  
      { 
        return value::make<number_value>(left->get_integer() + right->get_integer()); 
      }
      else if (oper == "-")  
      { 
        return value::make<number_value>(left->get_integer() - right->get_integer()); 
      }
      else if (oper == "*")  
      { 
        return value::make<number_value>(left->get_integer() * right->get_integer()); 
      }
      else if (oper == "/") 
      { 
        if (right->get_integer() == 0) {
          std::cerr <<  "[interpreter] Attempted division by zero encountered." << std::endl;
          return nullptr;
        }
        return value::make<number_value>(left->get_integer() / right->get_integer()); 
      }
      else if (oper == "%") 
      { 
        if (right->get_integer() == 0) {
          std::cerr <<  "[interpreter] Modulo with attempted division by zero encountered." 
                    <<  std::endl;
          return nullptr;
        }
        return value::make<number_value>(left->get_integer() % right->get_integer()); 
      }
      else if (oper == "&")  
        { return value::make<number_value>(left->get_integer() & right->get_integer()); }
      else if (oper == "|")  
        { return value::make<number_value>(left->get_integer() | right->get_integer()); }
      else if (oper == "^")  
        { return value::make<number_value>(left->get_integer() ^ right->get_integer()); }
      else if (oper == "<<")  
        { return value::make<number_value>(left->get_integer() << right->get_integer()); }
      else if (oper == ">>")  
        { return value::make<number_value>(left->get_integer() >> right->get_integer()); }
      else if (oper == "==")  
        { return value::make<number_value>(left->get_number() == right->get_number()); }
      else if (oper == "!=")  
        { return value::make<number_value>(left->get_number() != right->get_number()); }
      else if (oper == "<=")  
        { return value::make<number_value>(left->get_number() <= right->get_number()); }
      else if (oper == "<")  
        { return value::make<number_value>(left->get_number() < right->get_number()); }
      else if (oper == ">=")  
        { return value::make<number_value>(left->get_number() >= right->get_number()); }
      else if (oper == ">")  
        { return value::make<number_value>(left->get_number() > right->get_number()); }
      else if (oper == "&&")  
        { return value::make<number_value>(left->get_number() && right->get_number()); }
      else if (oper == "||")  
        { return value::make<number_value>(left->get_number() || right->get_number()); }
      
      std::cerr << "[interpreter] Invalid operation '" << oper
                << "' encountered in number vs number binary expression."
                << std::endl;
      return nullptr;
    }
  
    value::ptr address_vs_address (const value::ptr& lhs, const value::ptr& rhs,
      const std::string& oper)
    {
      auto left  = value_cast<address_value>(lhs);
      auto right = value_cast<address_value>(rhs);
      
      if (oper == "+")  
        { return value::make<address_value>(left->get_address() + right->get_address()); }
      else if (oper == "-")  
        { return value::make<address_value>(left->get_address() - right->get_address()); }
      else if (oper == "&")  
        { return value::make<address_value>(left->get_address() & right->get_address()); }
      else if (oper == "|")  
        { return value::make<address_value>(left->get_address() | right->get_address()); }
      else if (oper == "^")  
        { return value::make<address_value>(left->get_address() ^ right->get_address()); }
      else if (oper == "<<")  
        { return value::make<address_value>(left->get_address() << right->get_address()); }
      else if (oper == ">>")  
        { return value::make<address_value>(left->get_address() >> right->get_address()); }
      else if (oper == "==")  
        { return value::make<number_value>(left->get_address() == right->get_address()); }
      else if (oper == "!=")  
        { return value::make<number_value>(left->get_address() != right->get_address()); }
      else if (oper == "<=")  
        { return value::make<number_value>(left->get_address() <= right->get_address()); }
      else if (oper == "<")  
        { return value::make<number_value>(left->get_address() < right->get_address()); }
      else if (oper == ">=")  
        { return value::make<number_value>(left->get_address() >= right->get_address()); }
      else if (oper == ">")  
        { return value::make<number_value>(left->get_address() > right->get_address()); }
      else if (oper == "&&")  
        { return value::make<number_value>(left->get_address() && right->get_address()); }
      else if (oper == "||")  
        { return value::make<number_value>(left->get_address() || right->get_address()); }
      
      std::cerr << "[interpreter] Invalid operation '" << oper
                << "' encountered in address vs address binary expression."
                << std::endl;
      return nullptr;
    }
  
    value::ptr address_vs_number (const value::ptr& lhs, const value::ptr& rhs,
      const std::string& oper)
    {
      auto left  = value_cast<address_value>(lhs);
      auto right = value_cast<number_value>(rhs);
      
      if (oper == "+")  
        { return value::make<address_value>(left->get_address() + right->get_integer()); }
      else if (oper == "-")  
        { return value::make<address_value>(left->get_address() - right->get_integer()); }
      else if (oper == "==")  
        { return value::make<number_value>(left->get_address() == right->get_integer()); }
      else if (oper == "!=")  
        { return value::make<number_value>(left->get_address() != right->get_integer()); }
      else if (oper == "<=")  
        { return value::make<number_value>(left->get_address() <= right->get_integer()); }
      else if (oper == "<")  
        { return value::make<number_value>(left->get_address() < right->get_integer()); }
      else if (oper == ">=")  
        { return value::make<number_value>(left->get_address() >= right->get_integer()); }
      else if (oper == ">")  
        { return value::make<number_value>(left->get_address() > right->get_integer()); }
      else if (oper == "&&")  
        { return value::make<number_value>(left->get_address() && right->get_integer()); }
      else if (oper == "||")  
        { return value::make<number_value>(left->get_address() || right->get_integer()); }
      
      std::cerr << "[interpreter] Invalid operation '" << oper
                << "' encountered in address vs number binary expression."
                << std::endl;
      return nullptr;
    }
  
    value::ptr string_vs_string (const value::ptr& lhs, const value::ptr& rhs,
      const std::string& oper)
    {
      auto left  = value_cast<string_value>(lhs);
      auto right = value_cast<string_value>(rhs);
      
      if (oper == "+")  
      {
        return value::make<string_value>(left->get_string() + right->get_string()); 
      }
      else if (oper == "==")  
        { return value::make<number_value>(left->get_string() == right->get_string()); }
      else if (oper == "!=")  
        { return value::make<number_value>(left->get_string() != right->get_string()); }
      
      std::cerr << "[interpreter] Invalid operation '" << oper
                << "' encountered in string vs string binary expression."
                << std::endl;
      return nullptr;
    }
  
    value::ptr string_vs_number (const value::ptr& lhs, const value::ptr& rhs,
      const std::string& oper)
    {
      auto left  = value_cast<string_value>(lhs);
      auto right = value_cast<number_value>(rhs);
      
      if (oper == "+")  
      { 
        return value::make<string_value>(left->get_string() + 
          std::to_string(right->get_integer())); 
      }
      
      std::cerr << "[interpreter] Invalid operation '" << oper
                << "' encountered in string vs number binary expression."
                << std::endl;
      return nullptr;
    }
  
  }
  
  value::ptr interpreter::evaluate_binary_expression (const binary_expression* expr,
    environment& env)
  {
    auto lhs = evaluate(expr->get_left(), env);
    auto rhs = evaluate(expr->get_right(), env);
    if (lhs == nullptr || rhs == nullptr)
      { return nullptr; }
    
    if (
      lhs->get_value_type() == value_type::number &&
      rhs->get_value_type() == value_type::number
    ) { return priv::number_vs_number(lhs, rhs, expr->get_oper()); }
    else if (
      lhs->get_value_type() == value_type::address &&
      rhs->get_value_type() == value_type::address
    ) { return priv::address_vs_address(lhs, rhs, expr->get_oper()); }
    else if (
      lhs->get_value_type() == value_type::address &&
      rhs->get_value_type() == value_type::number
    ) { return priv::address_vs_number(lhs, rhs, expr->get_oper()); }
    else if (
      lhs->get_value_type() == value_type::number &&
      rhs->get_value_type() == value_type::address
    ) { return priv::address_vs_number(rhs, lhs, expr->get_oper()); }
    else if (
      lhs->get_value_type() == value_type::string &&
      rhs->get_value_type() == value_type::string
    ) { return priv::string_vs_string(lhs, rhs, expr->get_oper()); }
    else if (
      lhs->get_value_type() == value_type::string &&
      rhs->get_value_type() == value_type::number
    ) { return priv::string_vs_number(lhs, rhs, expr->get_oper()); }
    else if (
      lhs->get_value_type() == value_type::number &&
      rhs->get_value_type() == value_type::string
    ) { return priv::string_vs_number(rhs, lhs, expr->get_oper()); }

    return nullptr;
  }

  value::ptr interpreter::evaluate_address_expression (const address_literal* expr,
    environment& env)
  {
    auto addr_value = evaluate(expr->get_address_expr(), env);
    if (addr_value == nullptr)
    {
      return nullptr;
    }

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
