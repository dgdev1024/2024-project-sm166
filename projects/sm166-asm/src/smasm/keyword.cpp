/** @file smasm/keyword.cpp */

#include <smasm/keyword.hpp>

namespace smasm
{

  static const std::unordered_map<std::string, keyword> keyword_lookup = {
    { "", { keyword_type::none } },

    // Language Syntax
    { "def", { keyword_type::language, language_type::lt_def } },
    { "byte", { keyword_type::language, language_type::lt_byte } },
    { "word", { keyword_type::language, language_type::lt_word } },
    { "long", { keyword_type::language, language_type::lt_long } },

    // Directives
    { "size", { keyword_type::directive, directive_type::dt_size } },
    { "section", { keyword_type::directive, directive_type::dt_section } },

    // Sections
    { "rom", { keyword_type::section, section_type::st_rom } },
    { "ram", { keyword_type::section, section_type::st_ram } },

    // Restart and Interrupt Vectors
    { "rst0", { keyword_type::vector, vector_type::vt_rst0 } },
    { "rst1", { keyword_type::vector, vector_type::vt_rst1 } },
    { "rst2", { keyword_type::vector, vector_type::vt_rst2 } },
    { "rst3", { keyword_type::vector, vector_type::vt_rst3 } },
    { "rst4", { keyword_type::vector, vector_type::vt_rst4 } },
    { "rst5", { keyword_type::vector, vector_type::vt_rst5 } },
    { "rst6", { keyword_type::vector, vector_type::vt_rst6 } },
    { "rst7", { keyword_type::vector, vector_type::vt_rst7 } },
    { "int0", { keyword_type::vector, vector_type::vt_int0 } },
    { "int1", { keyword_type::vector, vector_type::vt_int1 } },
    { "int2", { keyword_type::vector, vector_type::vt_int2 } },
    { "int3", { keyword_type::vector, vector_type::vt_int3 } },
    { "int4", { keyword_type::vector, vector_type::vt_int4 } },
    { "int5", { keyword_type::vector, vector_type::vt_int5 } },
    { "int6", { keyword_type::vector, vector_type::vt_int6 } },
    { "int7", { keyword_type::vector, vector_type::vt_int7 } },

    // Jump Conditions
    { "n", { keyword_type::condition, condition_type::ct_n } },
    { "c", { keyword_type::condition, condition_type::ct_c } },
    { "nc", { keyword_type::condition, condition_type::ct_nc } },
    { "z", { keyword_type::condition, condition_type::ct_z } },
    { "nz", { keyword_type::condition, condition_type::ct_nz } },

    // CPU Registers
    { "b0", { keyword_type::cpu_register, register_type::rt_b0 } },
    { "b1", { keyword_type::cpu_register, register_type::rt_b1 } },
    { "b2", { keyword_type::cpu_register, register_type::rt_b2 } },
    { "b3", { keyword_type::cpu_register, register_type::rt_b3 } },
    { "b4", { keyword_type::cpu_register, register_type::rt_b4 } },
    { "b5", { keyword_type::cpu_register, register_type::rt_b5 } },
    { "b6", { keyword_type::cpu_register, register_type::rt_b6 } },
    { "b7", { keyword_type::cpu_register, register_type::rt_b7 } },
    { "b8", { keyword_type::cpu_register, register_type::rt_b8 } },
    { "b9", { keyword_type::cpu_register, register_type::rt_b9 } },
    { "b10", { keyword_type::cpu_register, register_type::rt_b10 } },
    { "b11", { keyword_type::cpu_register, register_type::rt_b11 } },
    { "b12", { keyword_type::cpu_register, register_type::rt_b12 } },
    { "b13", { keyword_type::cpu_register, register_type::rt_b13 } },
    { "b14", { keyword_type::cpu_register, register_type::rt_b14 } },
    { "b15", { keyword_type::cpu_register, register_type::rt_b15 } },
    { "w0", { keyword_type::cpu_register, register_type::rt_w0 } },
    { "w1", { keyword_type::cpu_register, register_type::rt_w1 } },
    { "w2", { keyword_type::cpu_register, register_type::rt_w2 } },
    { "w3", { keyword_type::cpu_register, register_type::rt_w3 } },
    { "w4", { keyword_type::cpu_register, register_type::rt_w4 } },
    { "w5", { keyword_type::cpu_register, register_type::rt_w5 } },
    { "w6", { keyword_type::cpu_register, register_type::rt_w6 } },
    { "w7", { keyword_type::cpu_register, register_type::rt_w7 } },
    { "l0", { keyword_type::cpu_register, register_type::rt_l0 } },
    { "l1", { keyword_type::cpu_register, register_type::rt_l1 } },
    { "l2", { keyword_type::cpu_register, register_type::rt_l2 } },
    { "l3", { keyword_type::cpu_register, register_type::rt_l3 } },

    // Instructions
    { "nop", { keyword_type::instruction, instruction_type::it_nop } },
    { "stop", { keyword_type::instruction, instruction_type::it_stop } },
    { "halt", { keyword_type::instruction, instruction_type::it_halt } },
    { "di", { keyword_type::instruction, instruction_type::it_di } },
    { "ei", { keyword_type::instruction, instruction_type::it_ei } },
    { "daa", { keyword_type::instruction, instruction_type::it_daa } },
    { "cpl", { keyword_type::instruction, instruction_type::it_cpl } },
    { "ccf", { keyword_type::instruction, instruction_type::it_ccf } },
    { "scf", { keyword_type::instruction, instruction_type::it_scf } },
    { "ld", { keyword_type::instruction, instruction_type::it_ld, 2 } },
    { "lhb", { keyword_type::instruction, instruction_type::it_lhb, 1 } },
    { "lhr", { keyword_type::instruction, instruction_type::it_lhr } },
    { "lhw", { keyword_type::instruction, instruction_type::it_lhw, 1 } },
    { "st", { keyword_type::instruction, instruction_type::it_st, 2 } },
    { "shb", { keyword_type::instruction, instruction_type::it_shb, 1 } },
    { "shr", { keyword_type::instruction, instruction_type::it_shr } },
    { "shw", { keyword_type::instruction, instruction_type::it_shw, 1 } },
    { "ssp", { keyword_type::instruction, instruction_type::it_ssp, 1 } },
    { "spc", { keyword_type::instruction, instruction_type::it_spc, 1 } },
    { "mv", { keyword_type::instruction, instruction_type::it_mv, 2 } },
    { "msp", { keyword_type::instruction, instruction_type::it_msp, 1 } },
    { "mpc", { keyword_type::instruction, instruction_type::it_mpc, 1 } },
    { "push", { keyword_type::instruction, instruction_type::it_push, 1 } },
    { "pop", { keyword_type::instruction, instruction_type::it_pop, 1 } },
    { "jmp", { keyword_type::instruction, instruction_type::it_jmp, 2 } },
    { "call", { keyword_type::instruction, instruction_type::it_call, 2 } },
    { "rst", { keyword_type::instruction, instruction_type::it_rst, 1 } },
    { "ret", { keyword_type::instruction, instruction_type::it_ret, 1 } },
    { "reti", { keyword_type::instruction, instruction_type::it_reti } },
    { "inc", { keyword_type::instruction, instruction_type::it_inc, 1 } },
    { "dec", { keyword_type::instruction, instruction_type::it_dec, 1 } },
    { "add", { keyword_type::instruction, instruction_type::it_add, 1 } },
    { "adc", { keyword_type::instruction, instruction_type::it_adc, 1 } },
    { "sub", { keyword_type::instruction, instruction_type::it_sub, 1 } },
    { "sbc", { keyword_type::instruction, instruction_type::it_sbc, 1 } },
    { "and", { keyword_type::instruction, instruction_type::it_and, 1 } },
    { "or", { keyword_type::instruction, instruction_type::it_or, 1 } },
    { "xor", { keyword_type::instruction, instruction_type::it_xor, 1 } },
    { "cmp", { keyword_type::instruction, instruction_type::it_cmp, 1 } },
    { "bit", { keyword_type::instruction, instruction_type::it_bit, 2 } },
    { "set", { keyword_type::instruction, instruction_type::it_set, 2 } },
    { "res", { keyword_type::instruction, instruction_type::it_res, 2 } },
    { "sla", { keyword_type::instruction, instruction_type::it_sla, 1 } },
    { "sra", { keyword_type::instruction, instruction_type::it_sra, 1 } },
    { "srl", { keyword_type::instruction, instruction_type::it_srl, 1 } },
    { "rl", { keyword_type::instruction, instruction_type::it_rl, 1 } },
    { "rla", { keyword_type::instruction, instruction_type::it_rla } },
    { "rlc", { keyword_type::instruction, instruction_type::it_rlc, 1 } },
    { "rlca", { keyword_type::instruction, instruction_type::it_rlca } },
    { "rr", { keyword_type::instruction, instruction_type::it_rr, 1 } },
    { "rra", { keyword_type::instruction, instruction_type::it_rra } },
    { "rrc", { keyword_type::instruction, instruction_type::it_rrc, 1 } },
    { "rrca", { keyword_type::instruction, instruction_type::it_rrca } }
  };

  const keyword& keyword::lookup (const std::string& name)
  {
    auto it = keyword_lookup.find(name);
    return (it != keyword_lookup.end()) ? it->second : keyword_lookup.at("");
  }

  const char* keyword::get_string_type () const
  {
    switch (type)
    {
      case keyword_type::none: return "none";
      case keyword_type::language: return "language";
      case keyword_type::directive: return "directive";
      case keyword_type::section: return "section";
      case keyword_type::vector: return "vector";
      case keyword_type::condition: return "condition";
      case keyword_type::cpu_register: return "cpu register";
      case keyword_type::instruction: return "instruction";
      default: return "unknown";
    }
  }

}
