/** @file smasm/instructions.cpp */

#include <smasm/interpreter.hpp>

namespace smasm
{

  #define has_one_arg(opc) \
    std::uint16_t opcode = opc; \
    auto arg_one = evaluate(stmt->get_first()); \
    if (arg_one == nullptr) { \
      std::cerr << "[instruction] Missing first argument to instruction '" \
                << stmt->get_mnemonic() << "'." << std::endl; \
      return false; \
    }

  #define has_two_args(opc) \
    has_one_arg(opc) \
    auto arg_two = evaluate(stmt->get_second()); \
    if (arg_two == nullptr) { \
      std::cerr << "[instruction] Missing second argument to instruction '" \
                << stmt->get_mnemonic() << "'." << std::endl; \
      return false; \
    }

  bool interpreter::evaluate_inst_ld (const instruction_statement* stmt)
  {
    has_two_args(0x1000);

    if (arg_one->get_value_type() != value_type::cpu_register) {
      std::cerr << "[instruction] Expected register for argument one of 'ld' instruction."
                << std::endl;
      return false;
    }

    auto dest_reg = value_cast<cpu_register_value>(arg_one);
    if (dest_reg->is_address_pointer() == true) {
      std::cerr << "[instruction] Expected non-pointer register for argument one of 'ld' instruction."
                << std::endl;
      return false;
    }

    switch (arg_two->get_value_type())
    {
      case value_type::number: {
        auto src_value = value_cast<number_value>(arg_two);
        if (dest_reg->is_byte_register()) {
          return  m_assembly.write_word(opcode + dest_reg->get_type()) &&
                  m_assembly.write_byte(src_value->get_integer() & 0xFF);
        } else if (dest_reg->is_word_register()) {
          return  m_assembly.write_word(opcode + dest_reg->get_type()) &&
                  m_assembly.write_word(src_value->get_integer() & 0xFFFF);
        } else {
          return  m_assembly.write_word(opcode + dest_reg->get_type()) &&
                  m_assembly.write_long(src_value->get_integer() & 0xFFFFFFFF);
        }
      } break;
      case value_type::address: {
        opcode += 0x20;
        auto src_address = value_cast<address_value>(arg_two);
        return  m_assembly.write_word(opcode + dest_reg->get_type()) &&
                m_assembly.write_long(src_address->get_address());
      } break;
      case value_type::cpu_register: {
        auto src_reg = value_cast<cpu_register_value>(arg_two);
        if (
          src_reg->is_address_pointer() == false ||
          src_reg->is_long_register() == false
        ) {
          std::cerr << "[instruction] Expected long register pointer for argument two of "
                    << "'ld r8, [r32]' instruction." << std::endl;
          return false;
        }

        switch (src_reg->get_type()) {
          case register_type::rt_l0: opcode += 0x30; break;
          case register_type::rt_l1: opcode += 0x40; break;
          case register_type::rt_l2: opcode += 0x50; break;
          case register_type::rt_l3: opcode += 0x60; break;
          default: return false;
        }

        return m_assembly.write_word(opcode + dest_reg->get_type());
      } break;
      default:
        std::cerr << "[instruction] Expected number, address, or register pointer for argument two "
                  << "of 'ld' instruction." << std::endl;
        return false;
    }
  }

  bool interpreter::evaluate_inst_lh (const instruction_statement* stmt)
  {
    if (stmt->get_mnemonic() == "lhb") {
      has_one_arg(0x1070);
      if (arg_one->get_value_type() != value_type::address) {
        std::cerr << "[instruction] Expected byte address for argument to 'lhb'." << std::endl;
        return false;
      }

      auto src_address = value_cast<address_value>(arg_one);
      return m_assembly.write_word(opcode) &&
             m_assembly.write_byte(src_address->get_address() & 0xFF);
    } else if (stmt->get_mnemonic() == "lhw") {
      has_one_arg(0x1072);
      if (arg_one->get_value_type() != value_type::address) {
        std::cerr << "[instruction] Expected word address for argument to 'lhw'." << std::endl;
        return false;
      }

      auto src_address = value_cast<address_value>(arg_one);
      return m_assembly.write_word(opcode) &&
             m_assembly.write_word(src_address->get_address() & 0xFFFF);
    } else {
      return m_assembly.write_word(0x1071);
    }
  }

  bool interpreter::evaluate_inst_st (const instruction_statement* stmt)
  {
    has_two_args(0x1100);

    auto src_register = value_cast<cpu_register_value>(arg_two);
    if (
      src_register->is_address_pointer() == true ||
      src_register->is_byte_register() == false
    ) {
      std::cerr << "[instruction] Expected byte register for argument one of instruction 'st'."
                << std::endl;
      return false;
    }

    switch (arg_one->get_value_type())
    {
      case value_type::address: {
        opcode += 0x20;
        auto dest_address = value_cast<address_value>(arg_one);
        
        return m_assembly.write_word(opcode + src_register->get_type()) &&
               m_assembly.write_long(dest_address->get_address());
      } break;
      case value_type::cpu_register: {
        auto dest_register = value_cast<cpu_register_value>(arg_one);
        if (
          dest_register->is_long_register() == false ||
          dest_register->is_address_pointer() == false
        ) {
          std::cerr << "[instruction] Expected register pointer for argument two of instruction "
                    << "'st [r32], r8'."
                    << std::endl;
          return false;
        }

        switch (dest_register->get_type()) {
          case register_type::rt_l0: opcode += 0x30; break;
          case register_type::rt_l1: opcode += 0x40; break;
          case register_type::rt_l2: opcode += 0x50; break;
          case register_type::rt_l3: opcode += 0x60; break;
          default: return false;
        }

        return m_assembly.write_word(opcode + src_register->get_type());
      } break;
      default:
        std::cerr << "[instruction] Expected address or register pointer for argument two of "
                  << "instruction 'st'."
                  << std::endl;
        return false;
    }
  }

  bool interpreter::evaluate_inst_sh (const instruction_statement* stmt)
  {
    switch (keyword::lookup(stmt->get_mnemonic()).param_one)
    {
      case instruction_type::it_shb: {
        has_one_arg(0x1170);
        if (arg_one->get_value_type() != value_type::address) {
          std::cerr << "[instruction] Expected address for parameter of instruction 'shb'."
                    << std::endl;
          return false;
        }

        return m_assembly.write_word(opcode) &&
               m_assembly.write_byte(value_cast<address_value>(arg_one)->get_address() & 0xFF);
      } break;
      case instruction_type::it_shr: {
        return m_assembly.write_word(0x1171);
      } break;
      case instruction_type::it_shw: {
        has_one_arg(0x1172);
        if (arg_one->get_value_type() != value_type::address) {
          std::cerr << "[instruction] Expected address for parameter of instruction 'shw'."
                    << std::endl;
          return false;
        }

        return m_assembly.write_word(opcode) &&
               m_assembly.write_word(value_cast<address_value>(arg_one)->get_address() & 0xFFFF);
      } break;
      case instruction_type::it_ssp: {
        has_one_arg(0x1173);
        if (arg_one->get_value_type() != value_type::address) {
          std::cerr << "[instruction] Expected address for parameter of instruction 'ssp'."
                    << std::endl;
          return false;
        }

        return m_assembly.write_word(opcode) &&
               m_assembly.write_long(value_cast<address_value>(arg_one)->get_address());
      } break;
      case instruction_type::it_spc: {
        has_one_arg(0x1174);
        if (arg_one->get_value_type() != value_type::address) {
          std::cerr << "[instruction] Expected address for parameter of instruction 'spc'."
                    << std::endl;
          return false;
        }

        return m_assembly.write_word(opcode) &&
               m_assembly.write_long(value_cast<address_value>(arg_one)->get_address());
      } break;
      default: return false;
    }
  }

  bool interpreter::evaluate_inst_mv (const instruction_statement* stmt)
  {
    has_two_args(0x1200);
    auto dest_reg = value_cast<cpu_register_value>(arg_one);
    auto src_reg  = value_cast<cpu_register_value>(arg_two);
    
    if (dest_reg->is_address_pointer() || src_reg->is_address_pointer())
    {
      std::cerr << "[instruction] Expected non-pointer registers for arguments of instruction 'mv'."
                << std::endl;
      return false;
    }

    if (dest_reg->is_byte_register() && src_reg->is_byte_register())
    {
      opcode += (0x10 * dest_reg->get_type());
      opcode += src_reg->get_type();

      return m_assembly.write_word(opcode);
    }
    else if (dest_reg->is_word_register() && src_reg->is_word_register())
    {
      opcode += 0x100;
      opcode += (0x10 * (dest_reg->get_type() - register_type::rt_w0));
      opcode += (src_reg->get_type() - register_type::rt_w0);

      return m_assembly.write_word(opcode);
    }
    else if (dest_reg->is_long_register() && src_reg->is_long_register())
    {
      opcode += 0x200;
      opcode += (0x10 * (dest_reg->get_type() - register_type::rt_l0));
      opcode += (src_reg->get_type() - register_type::rt_l0);

      return m_assembly.write_word(opcode);
    } else {
      std::cerr << "[instruction] Expected same-size registers for arguments of instruction 'mv'."
                << std::endl;
      return false;
    }
  }

  bool interpreter::evaluate_inst_ms (const instruction_statement* stmt)
  {
    has_one_arg(0x1500);

    auto dest_register = value_cast<cpu_register_value>(arg_one);
    if (dest_register->is_address_pointer()) {
      std::cerr << "[instruction] Expected non-pointer register for argument of instruction '"
                << stmt->get_mnemonic() << "'."
                << std::endl;
      return false;
    }

    switch (keyword::lookup(stmt->get_mnemonic()).param_one)
    {
      case instruction_type::it_msp: {
        if (dest_register->is_word_register() == false) {
          std::cerr << "[instruction] Expected word register for argument of instruction 'msp'."
                    << std::endl;
          return false;
        }

        return m_assembly.write_word(opcode + (dest_register->get_type() - register_type::rt_w0));
      } break;
      case instruction_type::it_mpc: {
        opcode += 0x08;
        if (dest_register->is_long_register() == false) {
          std::cerr << "[instruction] Expected long register for argument of instruction 'mpc'."
                    << std::endl;
          return false;
        }

        return m_assembly.write_word(opcode + (dest_register->get_type() - register_type::rt_l0));
      } break;
      default: return false;
    }
  }

  bool interpreter::evaluate_inst_push (const instruction_statement* stmt)
  {
    has_one_arg(0x1600);
    auto src_register = value_cast<cpu_register_value>(arg_one);
    if (src_register->is_address_pointer()) {
      std::cerr << "[instruction] Expected non-pointer register register for argument of "
                << "instruction 'push'."
                << std::endl;
      return false;
    }

    if (src_register->is_long_register()) {
      return m_assembly.write_word(opcode + src_register->get_type());
    } else {
      std::cerr << "[instruction] Expected long register register for argument of "
                << "instruction 'push'."
                << std::endl;
      return false;
    }
  }

  bool interpreter::evaluate_inst_pop (const instruction_statement* stmt)
  {
    has_one_arg(0x1620);
    auto src_register = value_cast<cpu_register_value>(arg_one);
    if (src_register->is_address_pointer()) {
      std::cerr << "[instruction] Expected non-pointer register register for argument of "
                << "instruction 'pop'."
                << std::endl;
      return false;
    }

    if (src_register->is_long_register()) {
      return m_assembly.write_word(opcode + src_register->get_type());
    } else {
      std::cerr << "[instruction] Expected long register for argument of "
                << "instruction 'pop'."
                << std::endl;
      return false;
    }
  }

  bool interpreter::evaluate_inst_jmp (const instruction_statement* stmt)
  {
    has_two_args(0x2000);

    if (arg_one->get_value_type() != value_type::cpu_condition) {
      std::cerr << "[instruction] Expected cpu condition for argument one of "
                << "instruction 'jmp'."
                << std::endl;
      return false;
    }

    auto condition = value_cast<cpu_condition_value>(arg_one);
    opcode += condition->get_type();

    switch (arg_two->get_value_type())
    {
      case value_type::address: {
        auto address = value_cast<address_value>(arg_two);
        return m_assembly.write_word(opcode) &&
               m_assembly.write_long(address->get_address());
      } break;
      case value_type::cpu_register: {
        opcode += 0x10;

        auto addr_reg = value_cast<cpu_register_value>(arg_two);
        if (addr_reg->is_address_pointer() == false || addr_reg->is_long_register() == false) {
          std::cerr << "[instruction] Expected long pointer register for argument two of "
                    << "instruction 'jmp cond, [r32]'." << std::endl;
          return false;
        }

        return m_assembly.write_word(
          opcode + (0x10 * (addr_reg->get_type() - register_type::rt_l0))
        );
      } break;
      default:
        std::cerr << "[instruction] Expected address or pointer register for argument two of "
                  << "instruction 'jmp'." << std::endl;
        return false;
    }
  }

  bool interpreter::evaluate_inst_call (const instruction_statement* stmt)
  {
    has_two_args(0x2200);

    if (arg_one->get_value_type() != value_type::cpu_condition) {
      std::cerr << "[instruction] Expected cpu condition for argument one of "
                << "instruction 'call'."
                << std::endl;
      return false;
    }

    auto condition = value_cast<cpu_condition_value>(arg_one);
    opcode += condition->get_type();

    if (arg_two->get_value_type() != value_type::address) {
      std::cerr << "[instruction] Expected address for argument two of "
                << "instruction 'call'." << std::endl;
      return false;
    }

    auto address = value_cast<address_value>(arg_two);
    return m_assembly.write_word(opcode) &&
           m_assembly.write_long(address->get_address());
  }

  bool interpreter::evaluate_inst_rst (const instruction_statement* stmt)
  {
    has_one_arg(0x2210);
    
    if (arg_one->get_value_type() != value_type::number) {
      std::cerr << "[instruction] Expected number for argument to 'rst'." << std::endl;
      return false;
    }

    const auto dest_val = value_cast<number_value>(arg_one);
    return m_assembly.write_word(opcode) && m_assembly.write_byte(dest_val->get_integer() & 0b111);
  }

  bool interpreter::evaluate_inst_ret (const instruction_statement* stmt)
  {
    has_one_arg(0x2300);

    if (arg_one->get_value_type() != value_type::cpu_condition) {
      std::cerr << "[instruction] Expected cpu condition for argument of "
                << "instruction 'ret'."
                << std::endl;
      return false;
    }

    auto condition = value_cast<cpu_condition_value>(arg_one);
    opcode += condition->get_type();

    return m_assembly.write_word(opcode);
  }

  bool interpreter::evaluate_inst_inc (const instruction_statement* stmt)
  {
    has_one_arg(0x3000);

    switch (arg_one->get_value_type())
    {

      case value_type::cpu_register: {
        
        auto reg = value_cast<cpu_register_value>(arg_one);
        if (reg->is_address_pointer() == false) {
          return m_assembly.write_word(opcode + reg->get_type());
        } else if (reg->is_long_register() == false) {
          std::cerr << "[instruction] Expected long register pointer for argument of instruction "
                    << "'inc [r32]'." << std::endl;
          return false;
        }

        opcode += 0x30;
        return m_assembly.write_word(opcode + (reg->get_type() - register_type::rt_l0));

      } break;

      case value_type::address: {

        auto addr = value_cast<address_value>(arg_one);
        opcode += 0x20;
        return m_assembly.write_word(opcode) &&
               m_assembly.write_long(addr->get_address());

      } break;

      default:
        std::cerr << "[instruction] Expected register, address or long register pointer for "
                  << "argument of instruction 'inc'." << std::endl;
        return false;

    }
  }

  bool interpreter::evaluate_inst_dec (const instruction_statement* stmt)
  {
    has_one_arg(0x3100);

    switch (arg_one->get_value_type())
    {

      case value_type::cpu_register: {
        
        auto reg = value_cast<cpu_register_value>(arg_one);
        if (reg->is_address_pointer() == false) {
          return m_assembly.write_word(opcode + reg->get_type());
        } else if (reg->is_long_register() == false) {
          std::cerr << "[instruction] Expected long register pointer for argument of instruction "
                    << "'dec [r32]'." << std::endl;
          return false;
        }

        opcode += 0x30;
        return m_assembly.write_word(opcode + (reg->get_type() - register_type::rt_l0));

      } break;

      case value_type::address: {

        auto addr = value_cast<address_value>(arg_one);
        opcode += 0x20;
        return m_assembly.write_word(opcode) &&
               m_assembly.write_long(addr->get_address());

      } break;

      default:
        std::cerr << "[instruction] Expected register, address or long register pointer for "
                  << "argument of instruction 'dec'." << std::endl;
        return false;

    }
  }

  bool interpreter::evaluate_inst_gen_a (const instruction_statement* stmt, std::uint16_t _opcode)
  {
    has_one_arg(_opcode);

    switch (arg_one->get_value_type())
    {
      case value_type::number: {
        return  m_assembly.write_word(opcode) &&
                m_assembly.write_byte(value_cast<number_value>(arg_one)->get_integer() & 0xFF);
      } break;
      case value_type::cpu_register: {
        auto reg = value_cast<cpu_register_value>(arg_one);
        if (reg->is_address_pointer() == true) {
          opcode += 0x30;
          if (reg->is_long_register() == false) {
            std::cerr << "[instruction] Expected long register pointer for argument to instruction "
                      << "'" << stmt->get_mnemonic() << " [r32]'" << std::endl;
            return false;
          }

          return m_assembly.write_word(opcode + (reg->get_type() - register_type::rt_l0));
        } else {
          opcode += 0x10;
          if (reg->is_byte_register() == false) {
            std::cerr << "[instruction] Expected byte register for argument to instruction "
                      << "'" << stmt->get_mnemonic() << " r8'" << std::endl;
            return false;
          }

          return m_assembly.write_word(opcode + reg->get_type());
        }
      } break;
      case value_type::address: {
        opcode += 0x20;
        auto addr = value_cast<address_value>(arg_one);
        return  m_assembly.write_word(opcode) &&
                m_assembly.write_long(addr->get_address());
      } break;
      default: {
        std::cerr << "[instruction] Expected integer, byte register, address or address pointer for "
                  << "argument to instruction '" << stmt->get_mnemonic() << "'." << std::endl;
        return false;
      } break;
    }
  }

  bool interpreter::evaluate_inst_gen_b (const instruction_statement* stmt, std::uint16_t _opcode)
  {
    has_one_arg(_opcode);

    switch (arg_one->get_value_type())
    {
      case value_type::cpu_register: {
        auto reg = value_cast<cpu_register_value>(arg_one);
        if (reg->is_address_pointer() == true) {
          opcode += 0x30;
          if (reg->is_long_register() == false) {
            std::cerr << "[instruction] Expected long register pointer for argument to instruction "
                      << "'" << stmt->get_mnemonic() << " [r32]'" << std::endl;
            return false;
          }

          return m_assembly.write_word(opcode + (reg->get_type() - register_type::rt_l0));
        } else {
          opcode += 0x10;
          if (reg->is_byte_register() == false) {
            std::cerr << "[instruction] Expected byte register for argument to instruction "
                      << "'" << stmt->get_mnemonic() << " r8'" << std::endl;
            return false;
          }

          return m_assembly.write_word(opcode + reg->get_type());
        }
      } break;
      case value_type::address: {
        opcode += 0x20;
        auto addr = value_cast<address_value>(arg_one);
        return  m_assembly.write_word(opcode) &&
                m_assembly.write_long(addr->get_address());
      } break;
      default: {
        std::cerr << "[instruction] Expected byte register, address or address pointer for "
                  << "argument to instruction '" << stmt->get_mnemonic() << "'." << std::endl;
        return false;
      } break;
    }
  }

  bool interpreter::evaluate_inst_gen_c (const instruction_statement* stmt, std::uint16_t _opcode)
  {
    has_two_args(_opcode);

    if (arg_one->get_value_type() != value_type::number) {
      std::cerr << "[instruction] Expected number for argument one of instruction "
                << "'" << stmt->get_mnemonic() << "'." << std::endl;
      return false;
    }

    auto bit = value_cast<number_value>(arg_one);
    switch (arg_two->get_value_type())
    {
      case value_type::cpu_register: {
        auto reg = value_cast<cpu_register_value>(arg_two);
        if (reg->is_address_pointer() == true) {
          opcode += 0x30;
          if (reg->is_long_register() == false) {
            std::cerr << "[instruction] Expected long register pointer for argument to instruction "
                      << "'" << stmt->get_mnemonic() << " bp [r32]'" << std::endl;
            return false;
          }

          return  m_assembly.write_word(opcode + (reg->get_type() - register_type::rt_l0)) &&
                  m_assembly.write_byte(bit->get_integer() & 0b111);
        } else {
          opcode += 0x10;
          if (reg->is_byte_register() == false) {
            std::cerr << "[instruction] Expected byte register for argument to instruction "
                      << "'" << stmt->get_mnemonic() << " bp r8'" << std::endl;
            return false;
          }

          return  m_assembly.write_word(opcode + reg->get_type()) &&
                  m_assembly.write_byte(bit->get_integer() & 0b111);
        }
      } break;
      case value_type::address: {
        opcode += 0x20;
        auto addr = value_cast<address_value>(arg_two);
        return  m_assembly.write_word(opcode) &&
                m_assembly.write_byte(bit->get_integer() & 0b111) &&
                m_assembly.write_long(addr->get_address());
      } break;
      default: {
        std::cerr << "[instruction] Expected byte register, address or address pointer for "
                  << "argument two to instruction '" << stmt->get_mnemonic() << "'." << std::endl;
        return false;
      } break;
    }
  }

  #undef has_two_args
  #undef has_one_arg

}
