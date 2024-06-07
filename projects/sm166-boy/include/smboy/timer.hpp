/** @file smboy/timer.hpp */

#pragma once

#include <smboy/common.hpp>

namespace smboy
{

  class emulator;

  enum class timer_speed
  {
    slowest = 0b00,
    fastest = 0b01,
    fast    = 0b10,
    slow    = 0b11
  };

  class timer
  {
  public:
    timer (emulator& _emulator);

  public:
    void tick ();

  public:
    inline bool is_enabled () const { return sm_getbit(m_control, 2); }
    inline timer_speed get_speed () const { return (timer_speed) (m_control & 0b11); }

  public:
    inline std::uint8_t read_reg_div  () const { return (m_divider >> 8) & 0xFF; }
    inline std::uint8_t read_reg_tima () const { return m_counter; }
    inline std::uint8_t read_reg_tma  () const { return m_modulo; }
    inline std::uint8_t read_reg_tac  () const { return m_control; }
    inline void         write_reg_div  () { m_divider = 0; }
    inline void         write_reg_tima (std::uint8_t value) { m_counter = value; }
    inline void         write_reg_tma  (std::uint8_t value) { m_modulo = value; }
    inline void         write_reg_tac  (std::uint8_t value) { m_control = value; }

  private:
    emulator& m_emulator;
    std::uint16_t m_divider = 0x000;
    std::uint8_t  m_counter = 0x00;
    std::uint8_t  m_modulo  = 0x00;
    std::uint8_t  m_control = 0x00;

  };

}
