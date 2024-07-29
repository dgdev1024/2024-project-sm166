/** @file smboy/emulator.cpp */

#include <smboy/emulator.hpp>

namespace smboy
{

  /** Public Methods ******************************************************************************/
  
  emulator& emulator::get_instance ()
  {
    static emulator s_instance;
    return s_instance;
  }
  
  void emulator::initialize ()
  {
    m_bus.initialize(this);
    m_timer.initialize(this);
    m_realtime.initialize(this);
    m_renderer.initialize(this);
    m_joypad.initialize(this);
    m_audio.initialize(this);
    m_ram.initialize();
    m_processor.initialize();
    m_processor.set_cycle_function(std::bind(&emulator::on_tick_cycle, this, std::placeholders::_1));
    m_running = true;
  }

  void emulator::stop ()
  {
    m_running = false;
  }
  
  bool emulator::step ()
  {
    bool result = m_processor.step(m_bus);
    if (m_processor.check_flag(sm::processor_flag_type::stop) == true)
    {
      m_running = false;
    }
    
    return result;
  }
  
  /** Tick Cycle Callback *************************************************************************/
  
  void emulator::on_tick_cycle (const std::uint64_t& cycle_count)
  {
    m_timer.tick();
    m_realtime.tick();
    m_renderer.tick(cycle_count);
    m_audio.tick(cycle_count, m_timer.audio_needs_update());
  }

}

