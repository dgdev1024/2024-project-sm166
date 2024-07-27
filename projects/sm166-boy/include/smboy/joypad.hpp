/** @file smboy/joypad.hpp */

#pragma once

#include <smboy/common.hpp>

namespace smboy
{

  class emulator;
  
  enum class joypad_button
  {
    a = 0,
    b,
    x,
    y,
    l,
    r,
    select,
    start
  };
  
  enum class joypad_dpad
  {
    up = 0,
    down,
    left,
    right
  };
  
  union joypad_control
  {
    struct
    {
      
      std::uint8_t enabled : 1;
      std::uint8_t buttons : 1;
      std::uint8_t dpad : 1;
      std::uint8_t : 1;
      std::uint8_t : 1;
      std::uint8_t : 1;
      std::uint8_t : 1;
      std::uint8_t : 1;
      
    };
    std::uint8_t state;
  };

  class joypad
  {
  
  public:
    void initialize (emulator* _emulator);
    
  public:
    void set_button (joypad_button button, bool pressed);
    void set_dpad (joypad_dpad dpad, bool pressed);
    
  public:
    std::uint8_t read_reg_joyb () const;
    std::uint8_t read_reg_joyd () const;
    std::uint8_t read_reg_joyc () const;
    
  public:
    void write_reg_joyc (std::uint8_t value);
    
  private:
    emulator* m_emulator = nullptr;
    std::uint8_t m_buttons = 0;
    std::uint8_t m_dpad = 0;
    joypad_control m_control;
  
  };

}

