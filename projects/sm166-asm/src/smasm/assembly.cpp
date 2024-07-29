/** @file smasm/assembly.cpp */

#include <smasm/assembly.hpp>

namespace smasm
{

  assembly::assembly ()
  {
    m_rom.resize(0x210, 0x00);
    m_charmaps[""] = {};
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
      m_rom.resize(m_rom.size() + 0x80);
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
  
  bool assembly::include_binary (const fs::path& path)
  {
    const auto& absolute = fs::absolute(path).lexically_normal();
    if (m_binary_files.contains(absolute) == true)
    {
      return true;
    }
    else
    {
      m_binary_files.insert(absolute);
    }
    
    std::fstream file { absolute, std::ios::in | std::ios::binary };
    if (file.is_open() == false) {
      std::cerr << "[assembly] Could not open binary file '" << absolute 
                << "' for reading." << std::endl;
      return false;
    }
    
    file.seekg(0, file.end);
    auto size = file.tellg();
    file.seekg(0, file.beg);
    
    std::uint8_t byte = 0;
    while (file.good())
    {
      file.read(reinterpret_cast<char*>(&byte), sizeof(std::uint8_t));
      if (write_byte(byte) == false)
      {
        return false;
      }
    }
    
    return true;
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

    std::cout << "[assembly] Wrote " << m_rom.size() << " bytes to rom file " << path
              << ".\n";
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
