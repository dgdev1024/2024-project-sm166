/** @file smboy/emulator.hpp */

#pragma once

#include <smboy/common.hpp>
#include <smboy/program.hpp>
#include <smboy/ram.hpp>
#include <smboy/bus.hpp>
#include <smboy/timer.hpp>
#include <smboy/realtime.hpp>
#include <smboy/renderer.hpp>
#include <sm/processor.hpp>

namespace smboy
{

  /**
   * @brief The @a `emulator` class is the main context of the `smboy` emulator.
   */
  class emulator
  {
  
  public:
  
    /**
     * @brief Retrieves the singleton instnace of the `smboy` emulator context, creating it if
     *        necessary.
     *
     * @return  A handle to the singleton instance.
     */
    static emulator& get_instance ();
    
    /**
     * @brief Initializes the singleton `smboy` emulator instance.
     */
    void initialize ();
    
    /**
     * @brief Brings the `smboy` emulator through one step of execution, ticking the CPU and the
     *        other components.
     *
     * @return  @a `true` if no errors occur during the tick;
     *          @a `false` otherwise.
     */
    bool step ();
    
  private:
    
    /**
     * @brief This function is called by the CPU on each tick cycle.
     *
     * @param cycle_count The number of tick cycles which have elapsed.
     */
    void on_tick_cycle (const std::uint64_t& cycle_count);
    
  public:
  
    /**
     * @brief Retrieves whether or not the emulator should continue running.
     *
     * @return  @a `true` if the emulator should continue running;
     *          @a `false` otherwise.
     */
    inline bool is_running () const { return m_running; }
    
    /**
     * @brief Retrieves the `smboy` emulator's external program data.
     *
     * @return  A handle to the external program data.
     */
    inline program& get_program () { return m_program; }
    inline const program& get_program () const { return m_program; }
    
    /**
     * @brief Retrieves the `smboy` emulator's internal memory.
     *
     * @return  A handle to the internal RAM.
     */
    inline ram& get_ram () { return m_ram; }
    inline const ram& get_ram () const { return m_ram; }
    
    inline timer& get_timer () { return m_timer; }
    inline const timer& get_timer () const { return m_timer; }

    inline realtime& get_realtime () { return m_realtime; }
    inline const realtime& get_realtime () const { return m_realtime; }
    
    inline renderer& get_renderer () { return m_renderer; }
    inline const renderer& get_renderer () const { return m_renderer; }
    
    /**
     * @brief Retrieves the `smboy` emulator's memory management unit (MMU).
     *
     * @return  A handle to the emulator's MMU.
     */
    inline bus& get_bus () { return m_bus; }
    inline const bus& get_bus () const { return m_bus; }
    
    /**
     * @brief Retrieves the SM166 CPU powering the `smboy` emulator.
     *
     * @return  A handle to the emulator's CPU.
     */
    inline sm::processor& get_processor () { return m_processor; }
    inline const sm::processor& get_processor () const { return m_processor; }
    
  private:
  
    /**
     * @brief Indicates whether or not the emulator should continue running.
     */
    bool m_running = false;
    
    /**
     * @brief Contains the `smboy` emulator's external program data.
     */
    program m_program;
    
    /**
     * @brief Contains the `smboy` emulator's internal RAM.
     */
    ram m_ram;

    timer m_timer;
    
    realtime m_realtime;
    
    renderer m_renderer;
    
    /**
     * @brief The `smboy` emulator's memory management unit (MMU).
     */
    bus m_bus;
  
    /**
     * @brief The SM166 CPU powering the `smboy` emulator.
     */
    sm::processor m_processor;
  
  };

}

