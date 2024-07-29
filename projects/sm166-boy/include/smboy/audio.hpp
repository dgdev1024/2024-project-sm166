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

  /** Common Channel Properties *******************************************************************/
  
  enum audio_envelope_direction
  {
    aed_decrease = 0,
    aed_increase = 1
  };

  union audio_phc_control
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
  
  union audio_ve_control
  {
    struct
    {
      std::uint8_t  sweep_pace      : 3;
      std::uint8_t  direction       : 1;
      std::uint8_t  initial_volume  : 4;
    };
    std::uint8_t state;
  };

  /** Pulse Channel Properties ********************************************************************/
  
  enum pulse_sweep_direction
  {
    psd_increase = 0,
    psd_decrease = 1
  };

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

  using pulse_ve_control  = audio_ve_control;
  using pulse_phc_control = audio_phc_control;
  
  /** Wave Channel Properties *********************************************************************/
  
  enum wave_output_level
  {
    wol_mute    = 0b00,
    wol_full    = 0b01,
    wol_half    = 0b10,
    wol_quarter = 0b11
  };

  union wave_ol_control
  {
    struct
    {
      std::uint8_t                : 5;
      std::uint8_t  output_level  : 2;
      std::uint8_t                : 1;
    };
    std::uint8_t state;
  };

  using wave_phc_control = audio_phc_control;

  /** Noise Channel Properties ********************************************************************/
  
  union noise_lt_control
  {
    struct
    {
      std::uint8_t  initial_length_timer  : 6;
      std::uint8_t                        : 2;
    };
    std::uint8_t state;
  };

  using noise_ve_control = audio_ve_control;

  union noise_lfsr_control
  {
    struct
    {
      std::uint8_t  divider     : 3;
      std::uint8_t  width       : 1;
      std::uint8_t  clock_shift : 4;
    };
    std::uint8_t state;
  };

  using noise_phc_control = audio_phc_control;

  /** Channel Structures **************************************************************************/
  
  struct pulse_channel
  {
    pulse_sweep_control psc;
    pulse_ld_control    ldc;
    pulse_ve_control    vec;
    std::uint8_t        plc;
    pulse_phc_control   phc;

    bool                dac_enable = false;
    std::uint8_t        dac_input = 0;
    float               dac_output = 0.0f;
    std::uint8_t        length_timer = 0;
    std::uint8_t        current_volume = 0;
    std::uint16_t       current_period = 0;
    std::uint16_t       period_divider = 0;
    std::uint8_t        frequency_sweep_ticks = 0;
    std::uint8_t        envelope_sweep_ticks = 0;
    std::uint8_t        wave_pointer = 0;

    inline std::uint16_t get_initial_period () const
    {
      return (plc | (phc.period_high << 8));
    }
  };
  
  struct wave_channel
  {
    std::uint8_t        dac;
    std::uint8_t        ilt;
    wave_ol_control     olc;
    std::uint8_t        plc;
    wave_phc_control    phc;
    std::uint8_t        ram[wave_ram_size];

    bool                dac_enable = false;
    std::uint8_t        dac_input = 0;
    float               dac_output = 0.0f;
    std::uint8_t        length_timer = 0;
    std::uint8_t        sample_index = 0;
    std::uint16_t       current_period = 0;
    std::uint16_t       period_divider = 0;

    inline std::uint16_t get_initial_period () const
    {
      return (plc | (phc.period_high << 8));
    }

    inline std::uint8_t read_wave_ram (const std::uint8_t address) const
    {
      return ram[address % wave_ram_size];
    }

    inline std::uint8_t read_wave_ram_nibble (const std::uint8_t address) const
    {
      if (address % 2 == 0)
        return (ram[(address / 2) % wave_ram_size] >> 4) & 0xF;
      else
        return (ram[(address / 2) % wave_ram_size]) & 0xF;
    }

    inline void write_wave_ram (const std::uint8_t address, std::uint8_t value)
    {

      ram[address % wave_ram_size] = value;
    }
  };

  struct noise_channel
  {
    noise_lt_control    ltc;
    noise_ve_control    vec;
    noise_lfsr_control  lfsr;
    noise_phc_control   phc;

    bool                dac_enable = false;
    std::uint8_t        dac_input = 0;
    float               dac_output = 0.0f;
    std::uint16_t       lfsr_state = 0;
    std::uint8_t        length_timer = 0;
    std::uint8_t        current_volume = 0;
    std::uint8_t        envelope_sweep_ticks = 0;
    std::uint64_t       clock_frequency = 0;
  };

  /** Audio Sample Structure **********************************************************************/
  
  struct audio_sample
  {
    std::uint8_t  left_input = 0;
    std::uint8_t  right_input = 0;
    float         left_output = 0.0f;
    float         right_output = 0.0f;
  };

  /** Audio Context Class *************************************************************************/
  
  class audio
  {
  
  public:  /** Public Methods *********************************************************************/
    
    void initialize (emulator* _emulator);
    void tick (const std::uint64_t& cycle_count, bool needs_update);
    audio_sample get_sample () const;
    
  public:  /** Register Reads *********************************************************************/
    inline std::uint8_t read_reg_nr10 () const { return m_pc1.psc.state; }
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
    inline std::uint8_t read_reg_nr32 () const { return m_wc.olc.state; }
    inline std::uint8_t read_reg_nr33 () const { return m_wc.plc; }
    inline std::uint8_t read_reg_nr34 () const { return m_wc.phc.state & 0b00111111; }
    inline std::uint8_t read_reg_nr41 () const { return m_nc.ltc.state; }
    inline std::uint8_t read_reg_nr42 () const { return m_nc.vec.state; }
    inline std::uint8_t read_reg_nr43 () const { return m_nc.lfsr.state; }
    inline std::uint8_t read_reg_nr44 () const { return m_nc.phc.state; }
    inline std::uint8_t read_reg_nr50 () const { return m_volume.state; }
    inline std::uint8_t read_reg_nr51 () const { return m_panning.state; }
    inline std::uint8_t read_reg_nr52 () const { return m_control.state; }
    
  public:  /** Register Writes ********************************************************************/
    inline void write_reg_nr10 (std::uint8_t value) { m_pc1.psc.state = value; }
    inline void write_reg_nr11 (std::uint8_t value) { m_pc1.ldc.state = value; }
    inline void write_reg_nr13 (std::uint8_t value) { m_pc1.plc = value; }
    inline void write_reg_nr21 (std::uint8_t value) { m_pc2.ldc.state = value; }
    inline void write_reg_nr23 (std::uint8_t value) { m_pc2.plc = value; }
    inline void write_reg_nr31 (std::uint8_t value) { m_wc.ilt = value; }
    inline void write_reg_nr32 (std::uint8_t value) { m_wc.olc.state = value; }
    inline void write_reg_nr33 (std::uint8_t value) { m_wc.plc = value; }
    inline void write_reg_nr41 (std::uint8_t value) { m_nc.ltc.state = value; }
    inline void write_reg_nr50 (std::uint8_t value) { m_volume.state = value; }
    inline void write_reg_nr51 (std::uint8_t value) { m_panning.state = value; }
    inline void write_reg_nr52 (std::uint8_t value) { m_control.state |= (value & 0b11110000); }
    
  public:  /** Register Writes With Side Effects **************************************************/
    void write_reg_nr12 (std::uint8_t value);
    void write_reg_nr14 (std::uint8_t value);
    void write_reg_nr22 (std::uint8_t value);
    void write_reg_nr24 (std::uint8_t value);
    void write_reg_nr30 (std::uint8_t value);
    void write_reg_nr34 (std::uint8_t value);
    void write_reg_nr42 (std::uint8_t value);
    void write_reg_nr43 (std::uint8_t value);
    void write_reg_nr44 (std::uint8_t value);

  public:  /** General Getters ********************************************************************/
  
    inline pulse_channel& get_pc1 () { return m_pc1; }
    inline const pulse_channel& get_pc1 () const { return m_pc1; }
    inline pulse_channel& get_pc2 () { return m_pc2; }
    inline const pulse_channel& get_pc2 () const { return m_pc2; }
    inline wave_channel& get_wc () { return m_wc; }
    inline const wave_channel& get_wc () const { return m_wc; }
    inline noise_channel& get_nc () { return m_nc; }
    inline const noise_channel& get_nc () const { return m_nc; }

  public:  /** General Setters ********************************************************************/

    inline void set_mix_clock (std::uint32_t frequency)
    {
      if (frequency == 0) { frequency = 44100; }
      m_mix_clock = 4194304 / frequency;
    }

    inline void set_mix_function
      (const std::function<void(const audio_sample&)>& on_mix)
    {
      m_on_mix = on_mix;
    }

  private: /** Ticking Methods ********************************************************************/
    void tick_length_timers ();
    void tick_frequency_sweep ();
    void tick_envelope_sweep ();
    void tick_wave_period_divider ();
    void tick_pulse_period_dividers ();
    void tick_noise_divider ();

  private: /** Hardware Registers *****************************************************************/
    pulse_channel m_pc1;
    pulse_channel m_pc2;
    wave_channel  m_wc;
    noise_channel m_nc;
    audio_control m_control;
    audio_panning m_panning;
    master_volume m_volume;

  private: /** Other Members **********************************************************************/
    std::uint16_t m_divider = 0;
    std::uint64_t m_mix_clock = 0;
    std::function<void(const audio_sample&)> m_on_mix = nullptr;

  private: /** Emulator Handle ********************************************************************/
    emulator* m_emulator = nullptr;
  
  };

}

