/** @file smasm/assembly.cpp */

#include <smasm/assembly.hpp>

namespace smasm
{

  assembly::assembly ()
  {
    m_rom.resize(0x200, 0x00);
  }

  bool assembly::resize_rom (const std::size_t size)
  {
    if (size <= m_rom.size()) {
      std::cerr <<  "[assembly] Attempted downwards resizing of ROM data buffer."
                <<  std::endl;
      return false;
    }

    m_rom.resize(size, 0x00);
    return true;
  }

  bool assembly::write_byte (std::uint8_t value)
  {
    if (m_in_ram == true) {
      std::cerr << "[assembly] Cannot write data outside of ROM mode." << std::endl;
      return false;
    } else if (m_rom_cursor >= m_rom.size()) {
      std::cerr << "[assembly] Not enough space in allocated ROM to write data." << std::endl;
      return false;
    }

    m_rom[m_rom_cursor++] = value;
    return true;
  }

  bool assembly::write_word (std::uint16_t value)
  {
    return  write_byte((value      ) & 0xFF) &&
            write_byte((value >>  8) & 0xFF);
  }

  bool assembly::write_long (std::uint32_t value)
  {
    return  write_byte((value      ) & 0xFF) &&
            write_byte((value >>  8) & 0xFF) &&
            write_byte((value >> 16) & 0xFF) &&
            write_byte((value >> 24) & 0xFF);
  }
  
  bool assembly::save_rom (const fs::path& path)
  {
    std::fstream file { path, std::ios::out | std::ios::binary };
    if (file.is_open() == false) {
      std::cerr << "[assembly] Could not open ROM file '" << path << "' for writing." << std::endl;
      return false;
    }

    file.write(reinterpret_cast<const char*>(m_rom.data()), m_rom.size());
    file.close();
    return true;
  }

  bool assembly::set_rom_cursor (const std::size_t cursor)
  {
    if (cursor < 0x100 && cursor % 0x10 != 0) {
      std::cerr <<  "[assembly] ROM cursor $" << std::hex << cursor 
                << " is not a valid interrupt or restart vector address."
                <<  std::endl;
      return false;
    }

    if (cursor >= m_rom.size()) {
      std::cerr <<  "[assembly] ROM cursor $" << std::hex << cursor << " is out of bounds."
                <<  std::endl;
      return false;
    }

    m_rom_cursor = cursor;
    return true;
  }

  void assembly::set_ram_cursor (std::size_t cursor)
  {
    if (cursor < 0x80000000) {
      cursor += 0x80000000;
    }

    m_ram_cursor = cursor;
  }

  void assembly::set_ram_mode (bool on)
  {
    m_in_ram = on;
  }

}
