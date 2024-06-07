/** @file smasm/assembly.hpp */

#pragma once

#include <smasm/common.hpp>

namespace smasm
{

  class assembly
  {
  public:
    assembly ();

  public:
    bool  resize_rom (const std::size_t size);
    bool  write_byte (std::uint8_t value);
    bool  write_word (std::uint16_t value);
    bool  write_long (std::uint32_t value);
    bool  save_rom (const fs::path& path);
    bool  set_rom_cursor (const std::size_t cursor);
    void  set_ram_cursor (std::size_t cursor);
    void  set_ram_mode (bool on);

  public:
    inline bool is_in_ram () const { return m_in_ram; }
    inline std::size_t get_rom_cursor () const { return m_rom_cursor; }
    inline std::size_t get_ram_cursor () const { return m_ram_cursor; }

  public:

    inline std::size_t get_current_cursor () const
    {
      return (m_in_ram == true) ? m_ram_cursor : m_rom_cursor;
    }

  private:
    std::vector<std::uint8_t> m_rom;
    std::size_t               m_rom_cursor  = 0x00000200;
    std::size_t               m_ram_cursor  = 0x80000000;
    bool                      m_in_ram      = false;

  };

}
