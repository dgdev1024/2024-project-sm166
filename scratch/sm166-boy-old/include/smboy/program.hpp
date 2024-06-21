/** @file smboy/program.hpp */

#pragma once

#include <smboy/common.hpp>

namespace smboy
{

  class program
  {
  public:
    bool load_file (const fs::path& path);
    bool load_sram ();
    bool save_sram ();
    std::uint8_t read_rom (std::uint32_t relative_address) const;
    std::uint8_t read_sram (std::uint32_t relative_address) const;
    void write_sram (std::uint32_t relative_address, std::uint8_t value);

  private:
    bool validate ();

  public:
    inline const std::vector<std::uint8_t>& get_rom () const { return m_rom; }
    inline const std::vector<std::uint8_t>& get_sram () const { return m_sram; }
    inline const std::string& get_title () const { return m_title; }
    inline const std::string& get_author () const { return m_author; }

  private:
    std::vector<std::uint8_t> m_rom;
    std::vector<std::uint8_t> m_sram;
    fs::path                  m_sram_path;
    std::string               m_title = "Untitled Program";
    std::string               m_author = "Unknown Author";

  };

}