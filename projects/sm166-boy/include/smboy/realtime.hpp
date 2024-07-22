/** @file smboy/realtime.hpp */

#pragma once

#include <smboy/common.hpp>

namespace smboy
{

  class emulator;

  class realtime
  {
  
  public:
    void initialize (emulator* _emulator);
    void tick ();

  public:
    inline bool is_enabled () const { return sm_getbit(m_control, 0); }

  public:
    inline std::uint8_t read_reg_rts  () const { return m_seconds; }
    inline std::uint8_t read_reg_rtm  () const { return m_minutes; }
    inline std::uint8_t read_reg_rth  () const { return m_hours; }
    inline std::uint8_t read_reg_rtdl () const { return m_days & 0xFF; }
    inline std::uint8_t read_reg_rtdh () const { return (m_days >> 8) & 0xFF; }
    inline std::uint8_t read_reg_rtc  () const { return m_control; }

    inline void write_reg_rtc (std::uint8_t value) { m_control = value; }

  private:
    emulator*     m_emulator = nullptr;
    std::uint16_t m_divider = 0x0000;
    std::uint8_t  m_seconds = 0x00;
    std::uint8_t  m_minutes = 0x00;
    std::uint8_t  m_hours = 0x00;
    std::uint16_t m_days = 0x00;
    std::uint8_t  m_control = 0x00;

  };

}
