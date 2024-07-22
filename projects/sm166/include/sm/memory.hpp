/** @file sm/memory.hpp */

#pragma once

#include <sm/common.hpp>

namespace sm
{

  /**
   * @brief The @a `memory` class is a base class for a memory management unit (MMU) through which
   *        the SM166 CPU can read and write data.
   */
  class memory
  {
  public:

    /**
     * @brief Reads one byte of data from the address bus at the given address.
     * 
     * @param address The 32-bit address from which the data is to be read.
     *  
     * @return  The value of the byte at the given address if successful;
     *          A user-defined error byte (eg. `0x00` or `0xFF`) otherwise. 
     */
    virtual std::uint8_t read_byte (std::uint32_t address) const = 0;

    /**
     * @brief Writes one byte of data to the address bus at the given address.
     * 
     * @param address The 32-bit address to which the data is to be written. 
     * @param value   The value of the byte to be written at that address.
     */
    virtual void write_byte (std::uint32_t address, std::uint8_t value) = 0;

  public:

    /**
     * @brief Pops a byte of data from the memory stack, adjusting the stack pointer as proper.
     * 
     * @param stack_pointer A handle to the CPU's stack pointer register.
     *  
     * @return  The value of the byte popped from the stack if successful;
     *          A user-defined error byte (eg. `0x00` or `0xFF`) otherwise. 
     */
    virtual std::uint8_t pop_byte (std::uint16_t& stack_pointer) const = 0;

    /**
     * @brief Pushes a byte of data into the memory stack, adjusting the stack pointer as proper.
     * 
     * @param stack_pointer A handle to the CPU's stack pointer register.
     * @param value         The value of the byte to be pushed into the stack.
     */
    virtual void push_byte (std::uint16_t& stack_pointer, std::uint8_t value) = 0;

  public:

    /**
     * @brief Reads one word (two bytes) of data from the address bus at the given address.
     * 
     * @param address The 32-bit address from which the data is to be read.
     *  
     * @return  The value of the bytes at the given address if successful;
     *          A user-defined error byte (eg. `0x00` or `0xFF`) otherwise. 
     */
    virtual std::uint16_t read_word (std::uint32_t address) const;

    /**
     * @brief Reads one long (four bytes) of data from the address bus at the given address.
     * 
     * @param address The 32-bit address from which the data is to be read.
     *  
     * @return  The value of the bytes at the given address if successful;
     *          A user-defined error byte (eg. `0x00` or `0xFF`) otherwise. 
     */
    virtual std::uint32_t read_long (std::uint32_t address) const;

    /**
     * @brief Writes one word (two bytes) of data to the address bus at the given address.
     * 
     * @param address The 32-bit address to which the data is to be written. 
     * @param value   The value of the bytes to be written at that address.
     */
    virtual void write_word (std::uint32_t address, std::uint16_t value);

    /**
     * @brief Writes one long (four bytes) of data to the address bus at the given address.
     * 
     * @param address The 32-bit address to which the data is to be written. 
     * @param value   The value of the bytes to be written at that address.
     */
    virtual void write_long (std::uint32_t address, std::uint32_t value);

  public:

    /**
     * @brief Pushes a word (two bytes) of data into the memory stack, adjusting the stack pointer 
     *        as proper.
     * 
     * @param stack_pointer A handle to the CPU's stack pointer register.
     * @param value         The value of the bytes to be pushed into the stack.
     */
    virtual void push_word (std::uint16_t& stack_pointer, std::uint16_t value);

    /**
     * @brief Pushes a long (four bytes) of data into the memory stack, adjusting the stack pointer 
     *        as proper.
     * 
     * @param stack_pointer A handle to the CPU's stack pointer register.
     * @param value         The value of the bytes to be pushed into the stack.
     */
    virtual void push_long (std::uint16_t& stack_pointer, std::uint32_t value);

    /**
     * @brief Pops a word (two bytes) of data from the memory stack, adjusting the stack pointer as 
     *        proper.
     * 
     * @param stack_pointer A handle to the CPU's stack pointer register.
     *  
     * @return  The value of the bytes popped from the stack if successful;
     *          A user-defined error value (eg. `0x00` or `0xFF`) otherwise. 
     */
    virtual std::uint16_t pop_word (std::uint16_t& stack_pointer) const;

    /**
     * @brief Pops a long (four bytes) of data from the memory stack, adjusting the stack pointer as 
     *        proper.
     * 
     * @param stack_pointer A handle to the CPU's stack pointer register.
     *  
     * @return  The value of the bytes popped from the stack if successful;
     *          A user-defined error value (eg. `0x00` or `0xFF`) otherwise. 
     */
    virtual std::uint32_t pop_long (std::uint16_t& stack_pointer) const;

  };

}
