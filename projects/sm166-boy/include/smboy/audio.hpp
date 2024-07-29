/** @file smboy/audio.hpp */

#pragma once

#include <smboy/common.hpp>

namespace smboy
{

  class emulator;
  
  /** Audio Control Union *************************************************************************/
  
  union audio_control
  {
    struct
    {
      std::uint8_t  pc1_enabled   : 1;
      std::uint8_t  pc2_enabled   : 1;
      std::uint8_t  wc_enabled    : 1;
      std::uint8_t  nc_enabled    : 1;
      std::uint8_t                : 3;
      std::uint8_t  master_enable : 1;
    };
    std::uint8_t state;
  };
  
  /** Audio Panning Union *************************************************************************/
  
  union audio_panning
  {
    struct
    {
      std::uint8_t  pc1_right     : 1;
      std::uint8_t  pc2_right     : 1;
      std::uint8_t  wc_right      : 1;
      std::uint8_t  nc_right      : 1;
      std::uint8_t  pc1_left      : 1;
      std::uint8_t  pc2_left      : 1;
      std::uint8_t  wc_left       : 1;
      std::uint8_t  nc_left       : 1;
    };
    std::uint8_t state;
  };
  
  /** Master Volume Union *************************************************************************/
  
  union master_volume
  {
    struct
    {
      std::uint8_t  right_volume  : 3;
      std::uint8_t  right_vin     : 1;
      std::uint8_t  left_volume   : 3;
      std::uint8_t  left_vin      : 1;
    };
    std::uint8_t state;
  };
  
  /** Pulse Channel Properties ********************************************************************/
  
  union pulse_sweep_control
  {
    struct
    {
      std::uint8_t  step      : 3;
      std::uint8_t  direction : 1;
      std::uint8_t  pace      : 3;
      std::uint8_t            : 1;
    };
    std::uint8_t state;
  };
  
  union pulse_ld_control
  {
    struct
    {
      std::uint8_t  initial_length_timer  : 6;
      std::uint8_t  wave_duty_cycle       : 2;
    };
    std::uint8_t state;
  };
  
  union pulse_ve_control
  {
    struct
    {
      std::uint8_t  sweep_pace      : 3;
      std::uint8_t  direction       : 1;
      std::uint8_t  initial_volume  : 4;
    };
    std::uint8_t state;
  };
  
  union pulse_phc_control
  {
    struct
    {
      std::uint8_t  period_high     : 3;
      std::uint8_t                  : 3;
      std::uint8_t  length_enable   : 1;
      std::uint8_t  trigger         : 1;
    };
    std::uint8_t state;
  };
  
  /** Wave Channel Properties *********************************************************************/
  
  
  
  /** Channel Structures **************************************************************************/
  
  struct pulse_channel
  {
    pulse_sweep_control psc;
    pulse_ld_control    ldc;
    pulse_ve_control    vec;
    std::uint8_t        plc;
    pulse_phc_control   phc;
  };
  
  struct wave_channel
  {
    std::uint8_t        dac;
    std::uint8_t        ilt;
  };
  
  /** Audio Context Class *************************************************************************/
  
  class audio
  {
  
  public:  /** Public Methods *********************************************************************/
    
    void initialize (emulator* _emulator);
    void tick (bool needs_update);
    
  public:  /** Register Reads *********************************************************************/
    inline std::uint8_t read_reg_nr10 () const { return m_pc1.pcs.state; }
    inline std::uint8_t read_reg_nr11 () const { return m_pc1.ldc.state; }
    inline std::uint8_t read_reg_nr12 () const { return m_pc1.vec.state; }
    inline std::uint8_t read_reg_nr13 () const { return m_pc1.plc; }
    inline std::uint8_t read_reg_nr14 () const { return m_pc1.phc.state & 0b00111111; }
    inline std::uint8_t read_reg_nr21 () const { return m_pc2.ldc.state; }
    inline std::uint8_t read_reg_nr22 () const { return m_pc2.vec.state; }
    inline std::uint8_t read_reg_nr23 () const { return m_pc2.plc; }
    inline std::uint8_t read_reg_nr24 () const { return m_pc2.phc.state & 0b00111111; }
    inline std::uint8_t read_reg_nr30 () const { return m_wc.dac; }
    inline std::uint8_t read_reg_nr31 () const { return m_wc.ilt; }
    inline std::uint8_t read_reg_nr50 () const { return m_volume.state; }
    inline std::uint8_t read_reg_nr51 () const { return m_panning.state; }
    inline std::uint8_t read_reg_nr52 () const { return m_control.state; }
    
  public:  /** Register Writes ********************************************************************/
    inline void write_reg_nr10 (std::uint8_t value) { m_pc1.pcs.state = value; }
    inline void write_reg_nr11 (std::uint8_t value) { m_pc1.ldc.state = value; }
    inline void write_reg_nr13 (std::uint8_t value) { m_pc1.plc = value; }
    inline void write_reg_nr21 (std::uint8_t value) { m_pc2.ldc.state = value; }
    inline void write_reg_nr23 (std::uint8_t value) { m_pc2.plc = value; }
    inline void write_reg_nr31 (std::uint8_t value) { m_wc.ilt = value; }
    inline void write_reg_nr50 (std::uint8_t value) { m_volume.state = value; }
    inline void write_reg_nr51 (std::uint8_t value) { m_panning.state = value; }
    inline void write_reg_nr52 (std::uint8_t value) { m_control.state |= (value & 0b11110000); }
    
  public:  /** Register Writes With Side Effects **************************************************/
    void write_reg_nr12 (std::uint8_t value);
    void write_reg_nr14 (std::uint8_t value);
    void write_reg_nr22 (std::uint8_t value);
    void write_reg_nr24 (std::uint8_t value);
    void write_reg_nr30 (std::uint8_t value);
  
  private: /** Hardware Registers *****************************************************************/
    pulse_channel m_pc1;
    pulse_channel m_pc2;
    wave_channel  m_wc;
    audio_control m_control;
    audio_panning m_panning;
    master_volume m_volume;
  
  private: /** Emulator Handle ********************************************************************/
    emulator* m_emulator = nullptr;
  
  };

}

