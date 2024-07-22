/** @file smboy/ram.hpp */

#pragma once

#include <smboy/common.hpp>

namespace smboy
{

  /**
   * @brief The @a `ram` class is responsible for storing the `smboy` emulator's internal random
   *        access data.
   */
  class ram
  {
  public:
  
    /**
     * @brief Initializes the emulator's internal RAM buffers, clearing them, resizing them, then
     *        zero-initializing them.
     */
    void initialize ();
    
    /**
     * @brief Reads a byte of data from the emulator's WRAM.
     *
     * @param address The address, relative to the start of WRAM in the address space, to read the
     *                byte from.
     *
     * @return  The value of the byte that was read if successful;
     *          `0xFF` if `address` is out of range.
     */
    std::uint8_t read_wram (std::uint32_t address) const;
    
    /**
     * @brief Reads a byte of data from the emulator's zero-page HRAM.
     *
     * @param address The address, relative to the start of HRAM in the address space, to read the
     *                byte from.
     *
     * @return  The value of the byte that was read if successful;
     *          `0xFF` if `address` is out of range.
     */
    std::uint8_t read_hram (std::uint32_t address) const;
    
    /**
     * @brief Reads a byte of data from the emulator's memory stack.
     *
     * @param address The address, relative to the start of the stack in the address space, to read 
     *                the byte from.
     *
     * @return  The value of the byte that was read if successful;
     *          `0xFF` if `address` is out of range.
     */
    std::uint8_t read_stack (std::uint32_t address) const;
    
    /**
     * @brief Writes a byte of data to the emulator's WRAM.
     *
     * @param address The address, relative to the start of WRAM in the address space, to read the
     *                byte from.
     * @param value   The value to be written to that address.
     *
     * @note  If `address` is out of range, then this method does nothing.
     */
    void write_wram (std::uint32_t address, std::uint8_t value);
    
    /**
     * @brief Writes a byte of data to the emulator's HRAM.
     *
     * @param address The address, relative to the start of HRAM in the address space, to read the
     *                byte from.
     * @param value   The value to be written to that address.
     *
     * @note  If `address` is out of range, then this method does nothing.
     */
    void write_hram (std::uint32_t address, std::uint8_t value);
    
    /**
     * @brief Writes a byte of data to the emulator's memory stack.
     *
     * @param address The address, relative to the start of the stack in the address space, to read 
     *                the byte from.
     * @param value   The value to be written to that address.
     *
     * @note  If `address` is out of range, then this method does nothing.
     */
    void write_stack (std::uint32_t address, std::uint8_t value);
  
  private:
  
    /**
     * @brief The emulator's internal working RAM (WRAM).
     */
    byte_buffer m_wram;
    
    /**
     * @brief The emulator's zero-page "high" RAM (HRAM).
     */
    byte_buffer m_hram;
    
    /**
     * @brief The emulator's memory stack.
     */
    byte_buffer m_stack;
  
  };

}

