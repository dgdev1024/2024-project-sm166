/** @file smasm/keyword.hpp */

#pragma once

#include <smasm/common.hpp>

namespace smasm
{

  enum class keyword_type
  {
    none,
    language,
    directive,
    section,
    vector,
    condition,
    cpu_register,
    instruction
  };

  enum language_type
  {
    lt_def,
    lt_byte,
    lt_word,
    lt_long,
    lt_repeat,
    lt_shift,
    lt_if,
    lt_else,
    lt_break,
    lt_include,
    lt_incbin,
    lt_let,
    lt_const,
    lt_global,
    lt_local,
    lt_function
  };

  enum directive_type
  {
    dt_size,
    dt_section
  };

  enum section_type
  {
    st_rom,
    st_ram
  };

  enum vector_type
  {
    vt_rst0,
    vt_rst1,
    vt_rst2,
    vt_rst3,
    vt_rst4,
    vt_rst5,
    vt_rst6,
    vt_rst7,
    vt_int0,
    vt_int1,
    vt_int2,
    vt_int3,
    vt_int4,
    vt_int5,
    vt_int6,
    vt_int7
  };

  enum condition_type
  {
    ct_n,
    ct_z,
    ct_nz,
    ct_c,
    ct_nc
  };

  enum register_type : int
  {
    rt_b0,
    rt_b1,
    rt_b2,
    rt_b3,
    rt_b4,
    rt_b5,
    rt_b6,
    rt_b7,
    rt_b8,
    rt_b9,
    rt_b10,
    rt_b11,
    rt_b12,
    rt_b13,
    rt_b14,
    rt_b15,
    rt_w0,
    rt_w1,
    rt_w2,
    rt_w3,
    rt_w4,
    rt_w5,
    rt_w6,
    rt_w7,
    rt_l0,
    rt_l1,
    rt_l2,
    rt_l3
  };

  enum instruction_type
  {
    it_nop,
    it_stop,
    it_halt,
    it_di,
    it_ei,
    it_daa,
    it_cpl,
    it_ccf,
    it_scf,
    it_ld,
    it_lhb,
    it_lhr,
    it_lhw,
    it_st,
    it_shb,
    it_shr,
    it_shw,
    it_ssp,
    it_spc,
    it_mv,
    it_msp,
    it_mpc,
    it_push,
    it_pop,
    it_jmp,
    it_call,
    it_rst,
    it_ret,
    it_reti,
    it_inc,
    it_dec,
    it_add,
    it_adc,
    it_sub,
    it_sbc,
    it_and,
    it_or,
    it_xor,
    it_cmp,
    it_bit,
    it_set,
    it_res,
    it_sla,
    it_sra,
    it_srl,
    it_rl,
    it_rla,
    it_rlc,
    it_rlca,
    it_rr,
    it_rra,
    it_rrc,
    it_rrca
  };

  struct keyword
  {
    keyword_type type = keyword_type::none;
    int param_one = 0;
    int param_two = 0;

    static const keyword& lookup (const std::string& name);
    const char* get_string_type () const;
  };

}
