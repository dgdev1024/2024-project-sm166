/** @file smboy/bus.cpp */

#include <smboy/emulator.hpp>
#include <smboy/bus.hpp>

namespace smboy
{

  void bus::initialize (emulator* _emulator)
  {
    m_emulator = _emulator;
  }

  std::uint8_t bus::read_byte (std::uint32_t address) const
  {
    if (m_emulator == nullptr)
    {
      return 0xFF;
    }
  
    if (address < rom_end_addr)
    {
      return m_emulator->get_program().read_rom(address);
    }
    
    else if (address >= wram_start_addr && address < wram_end_addr)
    {
      return m_emulator->get_ram().read_wram(address - wram_start_addr);
    }
    
    else if (address >= sram_start_addr && address < sram_end_addr)
    {
      return m_emulator->get_program().read_sram(address - sram_start_addr);
    }
    
    else if (address >= vram_start_addr && address < vram_end_addr)
    {
      return m_emulator->get_renderer().read_vram(address - vram_start_addr);
    }
    
    else if (address >= oam_start_addr && address < oam_end_addr)
    {
      return m_emulator->get_renderer().read_oam(address - oam_start_addr);
    }

    else if (address >= stack_start_addr && address < stack_end_addr)
    {
      return m_emulator->get_ram().read_stack(address - stack_start_addr);
    }
    
    else if (address >= hram_start_addr && address < hram_end_addr)
    {
      return m_emulator->get_ram().read_hram(address - hram_start_addr);
    }    
    
    else if (address >= io_start_addr) 
    {
      return read_io(address & 0xFF);
    }
    
    return 0xFF;
  }
  
  void bus::write_byte (std::uint32_t address, std::uint8_t value)
  {
    if (m_emulator == nullptr)
    {
      return;
    }
    
    if (address >= wram_start_addr && address < wram_end_addr)
    {
      m_emulator->get_ram().write_wram(address - wram_start_addr, value);
    }
    
    else if (address >= sram_start_addr && address < sram_end_addr)
    {
      m_emulator->get_program().write_sram(address - sram_start_addr, value);
    }
    
    else if (address >= vram_start_addr && address < vram_end_addr)
    {
      m_emulator->get_renderer().write_vram(address - vram_start_addr, value);
    }
    
    else if (address >= oam_start_addr && address < oam_end_addr)
    {
      m_emulator->get_renderer().write_oam(address - oam_start_addr, value);
    }
    
    else if (address >= stack_start_addr && address < stack_end_addr)
    {
      m_emulator->get_ram().write_stack(address - stack_start_addr, value);
    }
    
    else if (address >= hram_start_addr && address < hram_end_addr)
    {
      m_emulator->get_ram().write_hram(address - hram_start_addr, value);
    }
    
    else if (address >= io_start_addr) 
    {
      write_io(address & 0xFF, value);
    }
  }
  
  std::uint8_t bus::pop_byte (std::uint16_t& stack_pointer) const
  {
    return read_byte(stack_start_addr + (stack_pointer++));
  }
  
  void bus::push_byte (std::uint16_t& stack_pointer, std::uint8_t value)
  {
    write_byte(stack_start_addr + (--stack_pointer), value);
  }

  std::uint8_t bus::read_io (std::uint8_t address) const
  {
    switch (address) {
      case 0x00:  return m_emulator->get_joypad().read_reg_joyb();
      case 0x01:  return m_emulator->get_joypad().read_reg_joyd();
      case 0x02:  return m_emulator->get_joypad().read_reg_joyc();
      case 0x04:  return m_emulator->get_timer().read_reg_div();
      case 0x05:  return m_emulator->get_timer().read_reg_tima();
      case 0x06:  return m_emulator->get_timer().read_reg_tma();
      case 0x07:  return m_emulator->get_timer().read_reg_tac();
      case 0x08:  return m_emulator->get_realtime().read_reg_rts();
      case 0x09:  return m_emulator->get_realtime().read_reg_rtm();
      case 0x0A:  return m_emulator->get_realtime().read_reg_rth();
      case 0x0B:  return m_emulator->get_realtime().read_reg_rtdl();
      case 0x0C:  return m_emulator->get_realtime().read_reg_rtdh();
      case 0x0D:  return m_emulator->get_realtime().read_reg_rtc();
      case 0x0F:  return m_emulator->get_processor().get_interrupt_request();
      case 0x10:  return m_emulator->get_audio().read_reg_nr10();
      case 0x11:  return m_emulator->get_audio().read_reg_nr11();
      case 0x12:  return m_emulator->get_audio().read_reg_nr12();
      case 0x13:  return m_emulator->get_audio().read_reg_nr13();
      case 0x14:  return m_emulator->get_audio().read_reg_nr14();
      case 0x16:  return m_emulator->get_audio().read_reg_nr21();
      case 0x17:  return m_emulator->get_audio().read_reg_nr22();
      case 0x18:  return m_emulator->get_audio().read_reg_nr23();
      case 0x19:  return m_emulator->get_audio().read_reg_nr24();
      case 0x1A:  return m_emulator->get_audio().read_reg_nr30();
      case 0x1B:  return m_emulator->get_audio().read_reg_nr31();
      case 0x24:  return m_emulator->get_audio().read_reg_nr50();
      case 0x25:  return m_emulator->get_audio().read_reg_nr51();
      case 0x26:  return m_emulator->get_audio().read_reg_nr52();
      case 0x40:  return m_emulator->get_renderer().read_reg_lcdc();
      case 0x41:  return m_emulator->get_renderer().read_reg_stat();
      case 0x42:  return m_emulator->get_renderer().read_reg_scy();
      case 0x43:  return m_emulator->get_renderer().read_reg_scx();
      case 0x44:  return m_emulator->get_renderer().read_reg_ly();
      case 0x45:  return m_emulator->get_renderer().read_reg_lyc();
      case 0x49:  return m_emulator->get_renderer().read_reg_dma4();
      case 0x4A:  return m_emulator->get_renderer().read_reg_wy();
      case 0x4B:  return m_emulator->get_renderer().read_reg_wx();
      case 0x4F:  return m_emulator->get_renderer().read_reg_vbk();
      case 0x68:  return m_emulator->get_renderer().read_reg_bcps();
      case 0x69:  return m_emulator->get_renderer().read_reg_bcpd();
      case 0x6A:  return m_emulator->get_renderer().read_reg_obps();
      case 0x6B:  return m_emulator->get_renderer().read_reg_obpd();
      case 0x6C:  return m_emulator->get_renderer().read_reg_opri();
      case 0xFF:  return m_emulator->get_processor().get_interrupt_enable();
      default: return 0xFF;
    }
  }

  void bus::write_io (std::uint8_t address, std::uint8_t value)
  {
    switch (address) {
      case 0x02:  m_emulator->get_joypad().write_reg_joyc(value); break;
      case 0x04:  m_emulator->get_timer().write_reg_div(); break;
      case 0x05:  m_emulator->get_timer().write_reg_tima(value); break;
      case 0x06:  m_emulator->get_timer().write_reg_tma(value); break;
      case 0x07:  m_emulator->get_timer().write_reg_tac(value); break;
      case 0x0D:  m_emulator->get_realtime().write_reg_rtc(value); break;
      case 0x0F:  m_emulator->get_processor().set_interrupt_request(value); break;
      case 0x10:  m_emulator->get_audio().write_reg_nr10(value); break;
      case 0x11:  m_emulator->get_audio().write_reg_nr11(value); break;
      case 0x12:  m_emulator->get_audio().write_reg_nr12(value); break;
      case 0x13:  m_emulator->get_audio().write_reg_nr13(value); break;
      case 0x14:  m_emulator->get_audio().write_reg_nr14(value); break;
      case 0x16:  m_emulator->get_audio().write_reg_nr21(value); break;
      case 0x17:  m_emulator->get_audio().write_reg_nr22(value); break;
      case 0x18:  m_emulator->get_audio().write_reg_nr23(value); break;
      case 0x19:  m_emulator->get_audio().write_reg_nr24(value); break;
      case 0x1A:  m_emulator->get_audio().write_reg_nr30(value); break;
      case 0x1B:  m_emulator->get_audio().write_reg_nr31(value); break;
      case 0x24:  m_emulator->get_audio().write_reg_nr50(value); break;
      case 0x25:  m_emulator->get_audio().write_reg_nr51(value); break;
      case 0x26:  m_emulator->get_audio().write_reg_nr52(value); break;
      case 0x40:  m_emulator->get_renderer().write_reg_lcdc(value); break;
      case 0x41:  m_emulator->get_renderer().write_reg_stat(value); break;
      case 0x42:  m_emulator->get_renderer().write_reg_scy(value); break;
      case 0x43:  m_emulator->get_renderer().write_reg_scx(value); break;
      case 0x45:  m_emulator->get_renderer().write_reg_lyc(value); break;
      case 0x46:  m_emulator->get_renderer().write_reg_dma1(value); break;
      case 0x47:  m_emulator->get_renderer().write_reg_dma2(value); break;
      case 0x48:  m_emulator->get_renderer().write_reg_dma3(value); break;
      case 0x49:  m_emulator->get_renderer().write_reg_dma4(); break;
      case 0x4A:  m_emulator->get_renderer().write_reg_wy(value); break;
      case 0x4B:  m_emulator->get_renderer().write_reg_wx(value); break;
      case 0x4F:  m_emulator->get_renderer().write_reg_vbk(value); break;
      case 0x68:  m_emulator->get_renderer().write_reg_bcps(value); break;
      case 0x69:  m_emulator->get_renderer().write_reg_bcpd(value); break;
      case 0x6A:  m_emulator->get_renderer().write_reg_obps(value); break;
      case 0x6B:  m_emulator->get_renderer().write_reg_obpd(value); break;
      case 0x6C:  m_emulator->get_renderer().write_reg_opri(value); break;
      case 0xFF:  m_emulator->get_processor().set_interrupt_enable(value); break;
      default: break;
    }
  }
}
