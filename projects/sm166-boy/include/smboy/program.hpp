/** @file smboy/program.hpp */

#pragma once

#include <smboy/common.hpp>

namespace smboy
{

  /**
   * @brief The @a `program` is an emulator component which contains data loaded from an external
   *        program file. 
   */
  class program
  {
  public:
  
    /**
     * @brief Attempts to load a program file located at the given path.
     *
     * @param path  The path to the program file to be loaded.
     *
     * @return  @a `true` if the program file is loaded and validated successfully;
     *          @a `false` otherwise.
     */
    bool load_file (const fs::path& path);
    
    /**
     * @brief If a program file that calls for SRAM is loaded, then this method checks for an SRAM
     *        file coresponding to the loaded program and attempts to load it into the SRAM buffer.
     *
     * @return  @a `true` if the SRAM file is loaded successfully;
     *          @a `false` otherwise, or if the program file did not call for SRAM.
     */
    bool load_sram_file ();
    
    /**
     * @brief If the program currently loaded calls for SRAM, then this method is called every so
     *        often to save the contents of SRAM into an external file.
     *
     * @return  @a `true` if the SRAM file is saved successfully;
     *          @a `false` otherwise, or if the program file did not call for SRAM.
     */
    bool save_sram_file ();
    
    /**
     * @brief Reads a byte of data from the loaded program's ROM.
     *
     * @param address The address, relative to the start of ROM in the address space, to read the
     *                byte from.
     *
     * @return  The value of the byte that was read if successful;
     *          `0xFF` if `address` is out of range.
     */
    std::uint8_t read_rom (std::uint32_t address) const;
    
    /**
     * @brief Reads a byte of data from the loaded program's SRAM.
     *
     * @param address The address, relative to the start of SRAM in the address space, to read the
     *                byte from.
     *
     * @return  The value of the byte that was read if successful;
     *          `0xFF` if `address` is out of range, or if the loaded program did not call for SRAM.
     */
    std::uint8_t read_sram (std::uint32_t address) const;
    
    /**
     * @brief Writes a byte of data to the loaded program's SRAM.
     *
     * @param address The address, relative to the start of SRAM in the address space, to read the
     *                byte from.
     * @param value   The value to be written to that address.
     *
     * @note  If `address` is out of range, or if the loaded program did not call for SRAM, then
     *        this method does nothing.
     */
    void write_sram (std::uint32_t address, std::uint8_t value);
  
  private:
  
    /**
     * @brief Called as the program is being loaded, this method ensures that the file is a valid
     *        `smboy` program. Once validated, the program's title and author strings are set, and
     *        SRAM is allocated, if requested.
     *
     * @return  @a `true` if the program file is validated successfully;
     *          @a `false` otherwise.
     */
    bool validate ();
  
  public:
  
    /**
     * @brief Retrieves the program's read-only memory (ROM).
     *
     * @return  A handle to the program's ROM.
     */
    inline const byte_buffer& get_rom () const { return m_rom; }
  
    /**
     * @brief Retrieves the program's save memory (SRAM).
     *
     * @return  A handle to the program's SRAM.
     */
    inline const byte_buffer& get_sram () const { return m_sram; }
    
    /**
     * @brief Retrieves the program's title, contained in the program's header.
     *
     * @return  The program's title.
     */
    inline const std::string& get_title () const { return m_title; }
  
    /**
     * @brief Retrieves the name of the program's author(s), contained in the program's header.
     *
     * @return  The program's author(s).
     */
    inline const std::string& get_author () const { return m_author; }
  
  private:
  
    /**
     * @brief Contains the program's read-only memory (ROM). The ROM contains the instruction data
     *        which the `smboy` emulator's CPU needs to execute, as well as necessary asset data.
     */
    byte_buffer m_rom;
    
    /**
     * @brief Contains the program's save memory (SRAM). The SRAM contains data which is loaded from
     *        another separate file.
     */
    byte_buffer m_sram;
    
    /**
     * @brief This is the name of the binary file containing the SRAM data, if any, which needs to
     *        be loaded and automatically saved.
     *
     * @note  This path is deduced automatically once the program file is validated, if SRAM is to
     *        be allocated.
     */
    std::string m_sram_path = "";
    
    /**
     * @brief Contains the title of the program. This is contained in the program header section.
     */
    std::string m_title = "";
    
    /**
     * @brief Contains the name of the program's author. This is contained in the program header 
     *        section.
     */
    std::string m_author = "";
  
  };

}

