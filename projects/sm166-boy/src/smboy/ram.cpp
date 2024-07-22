/** @file smboy/ram.cpp */

#include <smboy/ram.hpp>

namespace smboy
{

  /** Public Methods ******************************************************************************/
  
  void ram::initialize ()
  {
    m_wram.clear();
    m_hram.clear();
    m_stack.clear();
    
    m_wram.resize(wram_size, 0x00);
    m_hram.resize(hram_size, 0x00);
    m_stack.resize(stack_size, 0x00);
  }
  
  std::uint8_t ram::read_wram (std::uint32_t address) const
  {
    if (address >= m_wram.size()) {
      std::cerr << "[ram] Relative WRAM address $" << std::hex << address
                << " is out of range." << std::endl;
      return 0xFF;
    }

    return m_wram.at(address);
  }
  
  std::uint8_t ram::read_hram (std::uint32_t address) const
  {
    if (address >= m_hram.size()) {
      std::cerr << "[ram] Relative HRAM address $" << std::hex << address
                << " is out of range." << std::endl;
      return 0xFF;
    }

    return m_hram.at(address);
  }
  
  std::uint8_t ram::read_stack (std::uint32_t address) const
  {
    if (address >= m_stack.size()) {
      std::cerr << "[ram] Relative stack address $" << std::hex << address
                << " is out of range." << std::endl;
      return 0xFF;
    }

    return m_stack.at(address);
  }
  
  void ram::write_wram (std::uint32_t address, std::uint8_t value)
  {
    if (address >= m_wram.size()) {
      std::cerr << "[ram] Relative WRAM address $" << std::hex << address
                << " is out of range." << std::endl;
      return;
    }

    m_wram[address] = value;
  }
  
  void ram::write_hram (std::uint32_t address, std::uint8_t value)
  {
    if (address >= m_hram.size()) {
      std::cerr << "[ram] Relative HRAM address $" << std::hex << address
                << " is out of range." << std::endl;
      return;
    }

    m_hram[address] = value;
  }
  
  void ram::write_stack (std::uint32_t address, std::uint8_t value)
  {
    if (address >= m_stack.size()) {
      std::cerr << "[ram] Relative stack address $" << std::hex << address
                << " is out of range." << std::endl;
      return;
    }

    m_stack[address] = value;
  }

}

