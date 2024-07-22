/** @file smboy/bus.hpp */

#pragma once

#include <smboy/common.hpp>
#include <sm/memory.hpp>

namespace smboy
{

  class emulator;

  /**
   * @brief The @a `bus` class is the `smboy` emulator's memory management unit (MMU).
   */
  class bus : public sm::memory
  {
  public:
  
    /**
     * @brief Initializes the memory bus with a pointer to the parent emulator.
     *
     * @param _emulator A pointer to the parent `smboy` emulator.
     */
    void initialize (emulator* _emulator);
  
  public:
    
    /**
     * @brief Reads one byte of data from the address bus at the given address.
     * 
     * @param address The 32-bit address from which the data is to be read.
     *  
     * @return  The value of the byte at the given address if successful;
     *          `0xFF` otherwise. 
     */
    std::uint8_t read_byte (std::uint32_t address) const override;

    /**
     * @brief Writes one byte of data to the address bus at the given address.
     * 
     * @param address The 32-bit address to which the data is to be written. 
     * @param value   The value of the byte to be written at that address.
     */
    void write_byte (std::uint32_t address, std::uint8_t value) override;

    /**
     * @brief Pops a byte of data from the memory stack, adjusting the stack pointer as proper.
     * 
     * @param stack_pointer A handle to the CPU's stack pointer register.
     *  
     * @return  The value of the byte popped from the stack if successful;
     *          `0xFF` otherwise. 
     */
    std::uint8_t pop_byte (std::uint16_t& stack_pointer) const override;

    /**
     * @brief Pushes a byte of data into the memory stack, adjusting the stack pointer as proper.
     * 
     * @param stack_pointer A handle to the CPU's stack pointer register.
     * @param value         The value of the byte to be pushed into the stack.
     */
    void push_byte (std::uint16_t& stack_pointer, std::uint8_t value) override;

  public:
    std::uint8_t read_io (std::uint8_t address) const;
    void write_io (std::uint8_t address, std::uint8_t value);
    
  private:
    emulator* m_emulator = nullptr;
  
  };

}

