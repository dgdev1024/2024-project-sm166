/** @file smboy/memory.cpp */

#include <smboy/emulator.hpp>
#include <smboy/memory.hpp>

namespace smboy
{
  
  memory::memory (emulator& emu) :
    m_emulator { emu }
  {
    m_wram.resize(wram_size);
    m_hram.resize(hram_size);
    m_stack.resize(stack_size);
  }

  std::uint8_t memory::read_byte (std::uint32_t address) const
  {
    if (address < rom_end_addr) {
      return m_emulator.get_program().read_rom(address);
    }
    else if (address >= wram_start_addr && address < wram_end_addr) {
      return m_wram.at(address - wram_start_addr);
    }
    else if (address >= sram_start_addr && address < sram_end_addr) {
      return m_emulator.get_program().read_sram(address - sram_start_addr);
    }
    else if (address >= stack_start_addr && address < stack_end_addr) {
      return m_stack.at(address - stack_start_addr);
    }
    else if (address >= hram_start_addr && address < hram_end_addr) {
      return m_hram.at(address - hram_start_addr);
    }
    else if (address >= io_start_addr) {
      return read_io(address & 0xFF);
    }
    
    return 0xFF;
  }

  void memory::write_byte (std::uint32_t address, std::uint8_t value)
  {
    if (address >= wram_start_addr && address < wram_end_addr) {
      m_wram[address - wram_start_addr] = value;
    }
    else if (address >= sram_start_addr && address < sram_end_addr) {
      m_emulator.get_program().write_sram(address - sram_start_addr, value);
    }
    else if (address >= stack_start_addr && address < stack_end_addr) {
      m_stack[address - stack_start_addr] = value;
    }
    else if (address >= hram_start_addr && address < hram_end_addr) {
      m_hram[address - hram_start_addr] = value;
    }
    else if (address >= io_start_addr) {
      write_io(address & 0xFF, value);
    }
  }

  void memory::push_byte (std::uint16_t& stack_pointer, std::uint8_t value)
  {
    write_byte(stack_start_addr + (--stack_pointer), value);
  }

  std::uint8_t memory::pop_byte (std::uint16_t& stack_pointer) const
  {
    return read_byte(stack_start_addr + (stack_pointer++));
  }

  std::uint8_t memory::read_io (std::uint8_t address) const
  {
    switch (address) {
      case 0x04:  return m_emulator.get_timer().read_reg_div();
      case 0x05:  return m_emulator.get_timer().read_reg_tima();
      case 0x06:  return m_emulator.get_timer().read_reg_tma();
      case 0x07:  return m_emulator.get_timer().read_reg_tac();
      case 0x0F:  return m_emulator.get_processor().get_interrupt_request();
      case 0xFF:  return m_emulator.get_processor().get_interrupt_enable();
      default: return 0xFF;
    }
  }

  void memory::write_io (std::uint8_t address, std::uint8_t value)
  {
    switch (address) {
      case 0x04:  m_emulator.get_timer().write_reg_div(); break;
      case 0x05:  m_emulator.get_timer().write_reg_tima(value); break;
      case 0x06:  m_emulator.get_timer().write_reg_tma(value); break;
      case 0x07:  m_emulator.get_timer().write_reg_tac(value); break;
      case 0x0F:  m_emulator.get_processor().set_interrupt_request(value); break;
      case 0xFF:  m_emulator.get_processor().set_interrupt_enable(value); break;
      default: break;
    }
  }

}
