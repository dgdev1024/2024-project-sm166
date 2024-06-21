/** @file smboy/realtime.cpp */

#include <smboy/emulator.hpp>
#include <smboy/realtime.hpp>

namespace smboy
{

  realtime::realtime (emulator& _emulator) :
    m_emulator { _emulator }
  {

  }

  void realtime::initialize ()
  {
    auto now          = std::chrono::system_clock::now().time_since_epoch();
    auto seconds      = std::chrono::duration_cast<std::chrono::seconds>(now).count();
    auto minutes      = std::chrono::duration_cast<std::chrono::minutes>(now).count();
    auto hours        = std::chrono::duration_cast<std::chrono::hours>(now).count();
    auto days         = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<86400>>>(now).count();

    m_seconds = (seconds % 60);
    m_minutes = (minutes % 60);
    m_hours   = (hours   % 24);
    m_days    = ((days & 0xFFFF) % 365);
  }

  void realtime::tick ()
  {
    std::uint16_t old_div = m_divider;
    m_divider++;

    if (is_enabled() == false) {
      return;
    }

    bool needs_update = (sm_getbit(old_div, 9) != 0) && (sm_getbit(m_divider, 9) == 0);
    if (needs_update == true) {
      auto now          = std::chrono::system_clock::now().time_since_epoch();
      auto seconds      = std::chrono::duration_cast<std::chrono::seconds>(now).count();
      auto minutes      = std::chrono::duration_cast<std::chrono::minutes>(now).count();
      auto hours        = std::chrono::duration_cast<std::chrono::hours>(now).count();
      auto days         = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<86400>>>(now).count();

      std::uint8_t old_seconds = m_seconds;
      m_seconds = (seconds % 60);
      m_minutes = (minutes % 60);
      m_hours   = (hours   % 24);
      m_days    = ((days & 0xFFFF) % 365);

      if (m_seconds != old_seconds) {
        m_emulator.get_processor().request_interrupt(interrupt_type::int_realtime);
      }
    }
  }

}
