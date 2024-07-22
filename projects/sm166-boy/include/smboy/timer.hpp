/** @file smboy/timer.hpp */

#pragma once

#include <smboy/common.hpp>

namespace smboy
{

  class emulator;

  enum timer_clock_speed
  {
    tcs_slowest = 0b00,
    tcs_fastest = 0b01,
    tcs_fast    = 0b10,
    tcs_slow    = 0b11
  };
  
  union timer_control
  {
    struct
    {
      std::uint8_t  clock_speed : 2;
      std::uint8_t  enabled     : 1;
      std::uint8_t              : 5;
    };
    
    std::uint8_t state;
  };

  /**
   * @brief The @a `timer` class is the `smboy` emulator's internal timer. This timer ticks at a
   *        configurable interval and requests an interrupt upon overflowing.
   */
  class timer
  {

  public:

    void initialize (emulator* _emulator);
    void tick ();
    
  public:
    
    inline std::uint8_t read_reg_div () const { return (m_divider >> 8) & 0xFF; }
    inline std::uint8_t read_reg_tima () const { return m_counter; }
    inline std::uint8_t read_reg_tma () const { return m_modulo; }
    inline std::uint8_t read_reg_tac () const { return m_control.state; }
    
    inline void write_reg_div () { m_divider = 0x0000; }
    inline void write_reg_tima (std::uint8_t value) { m_counter = value; }
    inline void write_reg_tma (std::uint8_t value) { m_modulo = value; }
    inline void write_reg_tac (std::uint8_t value) { m_control.state = value; }

  private:

    emulator*       m_emulator = nullptr;
    std::uint16_t   m_divider = 0x0000;
    std::uint8_t    m_counter = 0x00;
    std::uint8_t    m_modulo  = 0x00;
    timer_control   m_control;

  };

}
