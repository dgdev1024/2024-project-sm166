/** @file smboy/emulator.hpp */

#pragma once

#include <sm/processor.hpp>
#include <smboy/memory.hpp>
#include <smboy/program.hpp>
#include <smboy/timer.hpp>

namespace smboy
{

  class emulator
  {
  public:
    emulator (const fs::path& program_path);

  public:
    bool start ();

  private:
    void on_processor_cycle (const std::uint64_t& cycle_count);

  public:
    inline sm::processor& get_processor () { return m_processor; }
    inline const sm::processor& get_processor () const { return m_processor; }

    inline program& get_program () { return m_program; }
    inline const program& get_program () const { return m_program; }

    inline timer& get_timer () { return m_timer; }
    inline const timer& get_timer () const { return m_timer; }

  private:
    sm::processor m_processor;
    memory        m_memory;
    program       m_program;
    timer         m_timer;

  };

}
