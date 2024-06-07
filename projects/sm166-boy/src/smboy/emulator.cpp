/** @file smboy/emulator.cpp */

#include <smboy/emulator.hpp>

namespace smboy
{

  emulator::emulator (const fs::path& program_path) :
    m_memory { *this },
    m_timer { *this },
    m_realtime { *this }
  {
    m_program.load_file(program_path);
    m_processor.set_cycle_function(std::bind(
      &emulator::on_processor_cycle, this, std::placeholders::_1
    ));
    m_processor.initialize();
  }

  bool emulator::start ()
  {
    m_realtime.initialize();
    
    try {
      while (m_processor.check_flag(sm::processor_flag_type::stop) == false) {
        m_processor.step(m_memory);
      }
    } catch (...) {
      return false;
    }

    return true;
  }

  void emulator::on_processor_cycle (const std::uint64_t& cycle_count)
  {
    (void) cycle_count;

    m_timer.tick();
    m_realtime.tick();
  }

}
