/** @file sm/memory.cpp */

#include <sm/memory.hpp>

namespace sm
{

  std::uint16_t memory::read_word (std::uint32_t address) const
  {
    std::uint16_t first   = read_byte(address + 1);
    std::uint16_t second  = read_byte(address);

    return (
      (first << 8) |
      (second)
    );
  }

  std::uint32_t memory::read_long (std::uint32_t address) const
  {
    std::uint32_t first   = read_byte(address + 3);
    std::uint32_t second  = read_byte(address + 2);
    std::uint32_t third   = read_byte(address + 1);
    std::uint32_t fourth  = read_byte(address);

    return (
      (first  << 24) |
      (second << 16) |
      (third  <<  8) |
      (fourth)
    );
  }

  void memory::write_word (std::uint32_t address, std::uint16_t value)
  {
    write_byte(address    , (value >> 8) & 0xFF);
    write_byte(address + 1, (value     ) & 0xFF);
  }

  void memory::write_long (std::uint32_t address, std::uint32_t value)
  {
    write_byte(address    , (value >> 24) & 0xFF);
    write_byte(address + 1, (value >> 16) & 0xFF);
    write_byte(address + 2, (value >>  8) & 0xFF);
    write_byte(address + 3, (value      ) & 0xFF);
  }

  void memory::push_word (std::uint16_t& stack_pointer, std::uint16_t value)
  {
    push_byte(stack_pointer, (value     ) & 0xFF);
    push_byte(stack_pointer, (value >> 8) & 0xFF);
  }

  void memory::push_long (std::uint16_t& stack_pointer, std::uint32_t value)
  {
    push_byte(stack_pointer, (value      ) & 0xFF);
    push_byte(stack_pointer, (value >>  8) & 0xFF);
    push_byte(stack_pointer, (value >> 16) & 0xFF);
    push_byte(stack_pointer, (value >> 24) & 0xFF);
  }

  std::uint16_t memory::pop_word (std::uint16_t& stack_pointer) const
  {
    std::uint16_t first   = pop_byte(stack_pointer);
    std::uint16_t second  = pop_byte(stack_pointer);

    return (
      (first << 8) |
      (second)
    );
  }

  std::uint32_t memory::pop_long (std::uint16_t& stack_pointer) const
  {
    std::uint32_t first   = pop_byte(stack_pointer);
    std::uint32_t second  = pop_byte(stack_pointer);
    std::uint32_t third   = pop_byte(stack_pointer);
    std::uint32_t fourth  = pop_byte(stack_pointer);

    return (
      (first  << 24) |
      (second << 16) |
      (third  <<  8) |
      (fourth)
    );
  }

}
