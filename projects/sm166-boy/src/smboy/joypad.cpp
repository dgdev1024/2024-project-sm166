/** @file smboy/joypad.cpp */

#include <smboy/emulator.hpp>
#include <smboy/joypad.hpp>

namespace smboy
{

  void joypad::initialize (emulator* _emulator)
  {
    m_emulator = _emulator;
    m_control.enabled = 1;
    m_control.buttons = 1;
    m_control.dpad = 1;
  }
  
  void joypad::set_button (joypad_button button, bool pressed)
  {
    bool old_state = sm_getbit(m_buttons, (int) button);
    sm_setbit(m_buttons, (int) button, pressed);
    
    if (
      m_control.enabled == 1 &&
      m_control.buttons == 1 &&
      old_state == false &&
      pressed == true
    )
    {
      m_emulator->get_processor().request_interrupt(interrupt_type::int_joypad);
    }
  }
  
  void joypad::set_dpad (joypad_dpad dpad, bool pressed)
  {
    bool old_state = sm_getbit(m_dpad, (int) dpad);
    sm_setbit(m_dpad, (int) dpad, pressed);
    
    if (
      m_control.enabled == 1 &&
      m_control.dpad == 1 &&
      old_state == false &&
      pressed == true
    )
    {
      m_emulator->get_processor().request_interrupt(interrupt_type::int_joypad);
    }
  }
  
  std::uint8_t joypad::read_reg_joyb () const
  {
    if (m_control.enabled == 0 || m_control.buttons == 0)
    {
      return 0;
    }
    
    return m_buttons;
  }
  
  std::uint8_t joypad::read_reg_joyd () const
  {
    if (m_control.enabled == 0 || m_control.dpad == 0)
    {
      return 0;
    }
    
    return m_dpad;
  }
  
  std::uint8_t joypad::read_reg_joyc () const
  {
    return m_control.state;
  }
  
  void joypad::write_reg_joyc (std::uint8_t value)
  {
    m_control.state = value;
  }

}

