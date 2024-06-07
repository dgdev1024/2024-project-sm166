/** @file smboy/memory.hpp */

#pragma once

#include <smboy/common.hpp>
#include <sm/memory.hpp>

namespace smboy
{

  class emulator;

  class memory : public sm::memory
  {
  public:
    memory (emulator& emu);

  public:
    std::uint8_t read_byte (std::uint32_t address) const override;
    void write_byte (std::uint32_t address, std::uint8_t value) override;
    void push_byte (std::uint16_t& stack_pointer, std::uint8_t value) override;
    std::uint8_t pop_byte (std::uint16_t& stack_pointer) const override;

  public:
    std::uint8_t read_io (std::uint8_t address) const;
    void write_io (std::uint8_t address, std::uint8_t value);

  private:
    emulator& m_emulator;
    std::vector<std::uint8_t> m_wram;
    std::vector<std::uint8_t> m_hram;
    std::vector<std::uint8_t> m_stack;

  };

}
