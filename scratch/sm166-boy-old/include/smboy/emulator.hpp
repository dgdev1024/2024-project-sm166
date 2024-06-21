/** @file smboy/emulator.hpp */

#pragma once

#include <sm/processor.hpp>
#include <smboy/memory.hpp>
#include <smboy/program.hpp>
#include <smboy/timer.hpp>
#include <smboy/realtime.hpp>
#include <smboy/renderer.hpp>

namespace smboy
{

  class emulator
  {
  public:
    emulator (const fs::path& program_path);
    ~emulator ();

  public:
    bool start ();

  private:
    void on_processor_cycle (const std::uint64_t& cycle_count);

  public:
    inline sm::processor& get_processor () { return m_processor; }
    inline const sm::processor& get_processor () const { return m_processor; }

    inline memory& get_memory () { return m_memory; }
    inline const memory& get_memory () const { return m_memory; }

    inline program& get_program () { return m_program; }
    inline const program& get_program () const { return m_program; }

    inline timer& get_timer () { return m_timer; }
    inline const timer& get_timer () const { return m_timer; }

    inline realtime& get_realtime () { return m_realtime; }
    inline const realtime& get_realtime () const { return m_realtime; }

    inline renderer& get_renderer () { return m_renderer; }
    inline const renderer& get_renderer () const { return m_renderer; }

  private:
    sm::processor m_processor;
    memory        m_memory;
    program       m_program;
    timer         m_timer;
    realtime      m_realtime;
    renderer      m_renderer;

  };

}
