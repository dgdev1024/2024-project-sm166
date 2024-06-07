/** @file smboy/timer.cpp */

#include <smboy/emulator.hpp>
#include <smboy/timer.hpp>

namespace smboy
{

  timer::timer (emulator& _emulator) :
    m_emulator { _emulator }
  {

  }

  void timer::tick ()
  {
    std::uint16_t old_div = m_divider;
    m_divider++;

    if (is_enabled() == false) {
      return;
    }

    bool needs_update = false;
    switch (get_speed())
    {
      case timer_speed::slowest:
        needs_update = sm_getbit(old_div, 9) != 0 && sm_getbit(m_divider, 9) == 0;
        break;
      case timer_speed::fastest:
        needs_update = sm_getbit(old_div, 3) != 0 && sm_getbit(m_divider, 3) == 0;
        break;
      case timer_speed::fast:
        needs_update = sm_getbit(old_div, 5) != 0 && sm_getbit(m_divider, 5) == 0;
        break;
      case timer_speed::slow:
        needs_update = sm_getbit(old_div, 7) != 0 && sm_getbit(m_divider, 7) == 0;
        break;
    }

    if (needs_update == true) {
      if (++m_counter == 0xFF) {
        m_counter = m_modulo;
        m_emulator.get_processor().request_interrupt(interrupt_type::int_timer);
      }
    }
  }

}
