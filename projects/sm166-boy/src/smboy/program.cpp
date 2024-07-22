/** @file smboy/program.cpp */

#include <smboy/program.hpp>

namespace smboy
{

  /** Public Methods ******************************************************************************/
  
  bool program::load_file (const fs::path& path)
  {
  
    // Get the absolute form of the file's path, then load the program file.
    fs::path absolute = fs::absolute(path).lexically_normal();
    std::fstream file { absolute, std::ios::in | std::ios::binary };
    if (file.is_open() == false) {
      std::cerr <<  "[program] "
                <<  "Could not open program file '" << absolute << "' for reading." << std::endl;
      return false;
    }

    // Get the program file's size, then validate it.
    file.seekg(0, file.end);
    auto size = file.tellg();
    file.seekg(0, file.beg);

    // The minimum size of an SM166 ROM is 522 bytes (0x210).
    // The maximum size of an SM166 ROM is 64 megabytes (0x4000000).
    if (size < 0x210) {
      std::cerr <<  "[program] "
                <<  "Program file '" << absolute << "' is too small. "
                <<  "Minimum: " << 0x210 << " bytes." << std::endl;
      return false;
    } else if (size > 0x4000000) {
      std::cerr <<  "[program] "
                <<  "Program file '" << absolute << "' is too large. "
                <<  "Maximum: " << 0x4000000 << " bytes." << std::endl;
      return false;
    }

    // First, resize the ROM vector to fit only the first 512 bytes of the ROM.
    m_rom.resize(0x200);
    file.read(reinterpret_cast<char*>(m_rom.data()), 0x200);

    // The program header is stored at address $100. Validate the program's metadata.
    if (validate() == false) {
      std::cerr <<  "[program] "
                <<  "Program file '" << absolute << "' could not be validated."
                <<  std::endl;
      return false;
    }

    // If the program has SRAM allocated, then deduce the path to the program's SRAM file and load
    // that file.
    if (m_sram.size() != 0) {
      m_sram_path = absolute.string() + "-sram";
      load_sram_file();
    }

    // Resize the ROM vector to the full size of the program ROM, then load the full ROM in.
    m_rom.resize(size);
    file.seekg(0, file.beg);
    file.read(reinterpret_cast<char*>(m_rom.data()), size);
    file.close();

    std::cout << "[program] Program file: " << absolute << "\n"
              << "[program] Program title: \"" << m_title << "\"\n"
              << "[program] Program author: " << m_author << "\n";

    return true;
    
  }
  
  bool program::load_sram_file ()
  {
  
    // Don't bother attempting to load SRAM if the program does not call for it.
    if (m_sram.size() == 0 || m_sram_path.empty() == true) {
      return false;
    }

    // Attempt to load the SRAM file.
    std::fstream file { m_sram_path, std::ios::in | std::ios::binary };
    if (file.is_open() == false) {
      std::cerr <<  "[program] "
                <<  "Could not open SRAM file '" << m_sram_path << "' for reading." << std::endl;
      return false;
    }

    // Get the size of the SRAM file.
    file.seekg(0, file.end);
    auto size = file.tellg();
    file.seekg(0, file.beg);

    // Load the SRAM data. Don't load any more than 256 KB of SRAM.
    file.read(reinterpret_cast<char*>(m_sram.data()), m_sram.size());
    file.close();

    return true;
  
  }
  
  bool program::save_sram_file ()
  {
  
    // Don't bother attempting to save SRAM if the program does not call for it.
    if (m_sram.size() == 0 || m_sram_path.empty() == true) {
      return false;
    }

    // Open the SRAM file for writing.
    std::fstream file { m_sram_path, std::ios::in | std::ios::binary };
    if (file.is_open() == false) {
      std::cerr <<  "[program] "
                <<  "Could not open SRAM file '" << m_sram_path << "' for writing." << std::endl;
      return false;
    }

    // Save the current contents of SRAM to the file.
    file.write(reinterpret_cast<const char*>(m_sram.data()), m_sram.size());
    file.close();

    return true;
  
  }
  
  std::uint8_t program::read_rom (std::uint32_t address) const
  {
    if (address >= m_rom.size()) {
      std::cerr << "[program] Relative ROM address $" << std::hex << address
                << " is out of range." << std::endl;
      return 0xFF;
    }

    return m_rom.at(address);
  }
  
  std::uint8_t program::read_sram (std::uint32_t address) const
  {
    if (address >= m_sram.size()) {
      std::cerr << "[program] Relative SRAM address $" << std::hex << address
                << " is out of range." << std::endl;
      return 0xFF;
    }

    return m_sram.at(address);
  }
  
  void program::write_sram (std::uint32_t address, std::uint8_t value)
  {
    if (address >= m_sram.size()) {
      std::cerr << "[program] Relative SRAM address $" << std::hex << address
                << " is out of range." << std::endl;
      return;
    }

    m_sram[address] = value;
  }

  /** Program Validation **************************************************************************/
  
  bool program::validate ()
  {
  
    // First, get the program's magic number bytes, starting at $100.
    std::uint32_t magic_number = (
      (m_rom[0x103] << 24) |
      (m_rom[0x102] << 16) |
      (m_rom[0x101] <<  8) |
      (m_rom[0x100]      )
    );

    // Check to see if the magic number retrieved is correct.
    if (magic_number != 0x05316690) {
      std::cerr << "[program] Missing or incorrect magic number in program header." << std::endl;
      return false;
    }

    // Next, check for the program's title, which starts at $120 and shall be no greater than 32
    // bytes (0x20) in size.
    m_title.clear();
    for (std::size_t i = 0x120, c = 0; ; ++i, ++c) {

      // If we reach beyond 32 bytes without encountering the null terminator byte (0x00), then we
      // know the program title is too long, and therefore invalid.
      if (c == 32) {
        std::cerr << "[program] Title string in program header is too long."
                  << std::endl;
        return false;
      }

      // If this byte is 0x00 (the null terminator byte), then break.
      if (m_rom[i] == 0x00) { break; }

      // Ensure that the current title byte is a printable character.
      if (std::isprint(m_rom[i]) == 0) {
        std::cerr << "[program] Byte #" << c + 1 << " in program title is not a printable character."
                  << std::endl;
        return false;
      }

      // Insert the character.
      m_title += (char) m_rom[i];

    }

    // Next, check for the program's author, which starts right after the title string, at $140. As
    // with the title, it is also no greater than 32 bytes (0x20) in size.
    m_author.clear();
    for (std::size_t i = 0x140, c = 0; ; ++i, ++c) {

      // This process works just the same as the title string check above.

      if (c == 32) {
        std::cerr << "[program] Author string in program header is too long."
                  << std::endl;
        return false;
      }

      if (m_rom[i] == 0x00) { break; }

      if (std::isprint(m_rom[i]) == 0) {
        std::cerr << "[program] Byte #" << c + 1 << " in program author is not a printable character."
                  << std::endl;
        return false;
      }

      m_author += (char) m_rom[i];

    }
    
    // Perform any additional validation here, if necesssary.
    {

    }

    // Now that validation is done, while we're here, get the program's requested SRAM size, which
    // starts at $104. Resize the program's SRAM buffer to the size retrieved.
    std::uint32_t sram_size = (
      (m_rom[0x107] << 24) |
      (m_rom[0x106] << 16) |
      (m_rom[0x105] <<  8) |
      (m_rom[0x104]      )
    );
    if (sram_size != 0) {
      m_sram.resize(sram_size);
    }

    return true;
  
  }

}

