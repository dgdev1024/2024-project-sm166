/** @file smboy/timer.cpp */

#include <smboy/emulator.hpp>
#include <smboy/timer.hpp>

namespace smboy
{

  void timer::initialize (emulator* _emulator)
  {
    m_emulator = _emulator;
    m_divider = 0x0000;
    m_counter = 0x00;
    m_modulo = 0x00;
    m_control.state = 0xF8;
  }

  void timer::tick ()
  {
    
    // Don't bother ticking if the timer has no parent emulator.
    if (m_emulator == nullptr) { return; }
    
    // Get a handle to the emulator's processor. Don't bother ticking if the stop flag is set.
    sm::processor& cpu = m_emulator->get_processor();
    if (cpu.check_flag(sm::processor_flag_type::stop) == true)
    {
      return;
    }
    
    // Increment the timer's counter. Keep a copy of the old divider before doing so.
    std::uint16_t old_divider = (m_divider++);
    
    // Check to see if the audio context needs to be updated.
    m_div_apu = (sm_getbit(old_divider, 11) != 0) && (sm_getbit(m_divider, 11) == 0);
    
    // Don't bother updating the timer's counter if it's currently disabled.
    if (m_control.enabled == 0) { return; }
    
    // The timer's clock speed setting dictates which bit of the divider we check to determine
    // whether or not we update the timer's counter.
    std::uint8_t check_bit = 0;
    switch (m_control.clock_speed)
    {
      case timer_clock_speed::tcs_slowest:  check_bit = 9; break;
      case timer_clock_speed::tcs_fastest:  check_bit = 3; break;
      case timer_clock_speed::tcs_fast:     check_bit = 5; break;
      case timer_clock_speed::tcs_slow:     check_bit = 7; break;
      default:                              check_bit = 9; break;
    }
    
    // The timer's counter needs to be updated if the target bit in the divider was set in the
    // previous tick, but clear in the current tick.
    bool timer_update = (sm_getbit(old_divider, check_bit) != 0) &&
      (sm_getbit(m_divider, check_bit) == 0);
    if (timer_update == true && ++m_counter == 0xFF)
    {
      
      // If incrementing the counter causes it to overflow, then reset it to the modulo value and
      // request a CPU interrupt.
      m_counter = m_modulo;
      cpu.request_interrupt(interrupt_type::int_timer);
      
    }
    
  }

}
