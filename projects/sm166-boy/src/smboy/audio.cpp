/** @file smboy/audio.hpp */

#include <smboy/emulator.hpp>
#include <smboy/audio.hpp>

namespace smboy
{

  /** Private Constants - Wave Duty Patterns  *****************************************************/

  static constexpr std::uint8_t wave_duty_patterns[4] = {
    0b00000001, 0b00000011, 0b00001111, 0b00111111
  };

  /** Public Methods - Initialization *************************************************************/
  
  void audio::initialize (emulator* _emulator)
  {
    m_emulator = _emulator;

    write_reg_nr10(0x00);
    write_reg_nr11(0x00);
    write_reg_nr12(0x00);
    write_reg_nr13(0x00);
    write_reg_nr14(0x00);

    write_reg_nr21(0x00);
    write_reg_nr22(0x00);
    write_reg_nr23(0x00);
    write_reg_nr24(0x00);

    write_reg_nr30(0x00);
    write_reg_nr31(0x00);
    write_reg_nr32(0x00);
    write_reg_nr33(0x00);
    write_reg_nr34(0x00);

    write_reg_nr41(0x00);
    write_reg_nr42(0x00);
    write_reg_nr43(0x00);
    write_reg_nr44(0x00);

    write_reg_nr50(0x00);
    write_reg_nr51(0x00);
    write_reg_nr52(0x00);

    set_mix_clock(44100);
  }
  
  /** Public Methods - Tick ***********************************************************************/
  
  void audio::tick (const std::uint64_t& cycle_count, bool needs_update)
  {
    if (m_control.master_enable == false) { return; }

    if (cycle_count % 2 == 0) { tick_wave_period_divider(); }
    if (cycle_count % 4 == 0) { tick_pulse_period_dividers(); }
    if (cycle_count % m_nc.clock_frequency == 0)
      { tick_noise_divider(); }

    if (needs_update == true)
    {
      m_divider++;
      if (m_divider % 2 == 0) { tick_length_timers(); }
      if (m_divider % 4 == 0) { tick_frequency_sweep(); }
      if (m_divider % 8 == 0) { tick_envelope_sweep(); }
    }

    if (cycle_count % m_mix_clock == 0 && m_on_mix != nullptr)
      { m_on_mix(get_sample()); }
  }

  /** Public Methods - Mixing *********************************************************************/

  audio_sample audio::get_sample () const
  {
    audio_sample sample;

    if (m_control.pc1_enabled && m_pc1.dac_enable)
    {
      if (m_panning.pc1_left)  
        { sample.left_input += m_pc1.dac_input;
          sample.left_output += m_pc1.dac_output; }
      if (m_panning.pc1_right) 
        { sample.right_input += m_pc1.dac_input;
          sample.right_output += m_pc1.dac_output; }
    }

    if (m_control.pc2_enabled && m_pc2.dac_enable)
    {
      if (m_panning.pc2_left)  
        { sample.left_input += m_pc2.dac_input;
          sample.left_output += m_pc2.dac_output; }
      if (m_panning.pc2_right) 
        { sample.right_input += m_pc2.dac_input;
          sample.right_output += m_pc2.dac_output; }
    }

    if (m_control.wc_enabled && m_wc.dac_enable)
    {
      if (m_panning.wc_left)  
        { sample.left_input += m_wc.dac_input;
          sample.left_output += m_wc.dac_output; }
      if (m_panning.wc_right) 
        { sample.right_input += m_wc.dac_input;
          sample.right_output += m_wc.dac_output; }
    }

    if (m_control.nc_enabled && m_nc.dac_enable)
    {
      if (m_panning.nc_left)  
        { sample.left_input += m_nc.dac_input;
          sample.left_output += m_nc.dac_output; }
      if (m_panning.nc_right) 
        { sample.right_input += m_nc.dac_input;
          sample.right_output += m_nc.dac_output; }
    }

    sample.left_input /= 4;
    sample.right_input /= 4;
    sample.left_output /= 4.0f;
    sample.right_output /= 4.0f;

    return sample;
  }

  /** Public Methods - Register Writes With Side Effects ******************************************/
  
  void audio::write_reg_nr12 (std::uint8_t value)
  {
    m_pc1.vec.state = value;
    if ((m_pc1.vec.state & 0b11111000) == 0)
    {
      m_pc1.dac_enable = false;
      m_control.pc1_enabled = false;
    }
    else
    {
      m_pc1.dac_enable = true;
    }
  }
  
  void audio::write_reg_nr14 (std::uint8_t value)
  {
    m_pc1.phc.state = value;
    if (m_pc1.phc.trigger)
    {
      m_pc1.length_timer = m_pc1.ldc.initial_length_timer;
      m_pc1.current_period = m_pc1.get_initial_period();
      m_pc1.current_volume = m_pc1.vec.initial_volume;
      m_pc1.wave_pointer = 0;
      m_control.pc1_enabled = m_pc1.dac_enable;
    }
  }
  
  void audio::write_reg_nr22 (std::uint8_t value)
  {
    m_pc2.vec.state = value;
    if ((m_pc2.vec.state & 0b11111000) == 0)
    {
      m_pc2.dac_enable = false;
      m_control.pc2_enabled = false;
    }
    else
    {
      m_pc2.dac_enable = true;
    }
  }
  
  void audio::write_reg_nr24 (std::uint8_t value)
  {
    m_pc2.phc.state = value;
    
    if (m_pc2.phc.trigger)
    {
      m_pc2.length_timer = m_pc2.ldc.initial_length_timer;
      m_pc2.current_period = m_pc2.get_initial_period();
      m_pc2.current_volume = m_pc2.vec.initial_volume;
      m_pc2.wave_pointer = 0;
      m_control.pc2_enabled = m_pc2.dac_enable;
    }
  }
  
  void audio::write_reg_nr30 (std::uint8_t value)
  {
    m_wc.dac = value;
    m_wc.dac_enable = (m_wc.dac & 0b10000000);
    if (m_wc.dac_enable == false)
    {
      m_control.wc_enabled = false;
    }
  }

  void audio::write_reg_nr34 (std::uint8_t value)
  {
    m_wc.phc.state = value;
    
    if ((m_wc.phc.state & 0b10000000) == 1)
    {
      m_wc.length_timer = m_wc.ilt;
      m_wc.current_period = m_wc.get_initial_period();
      m_control.wc_enabled = m_wc.dac_enable;
    }
  }

  void audio::write_reg_nr42 (std::uint8_t value)
  {
    m_nc.vec.state = value;
    if ((m_nc.vec.state & 0b11111000) == 0)
    {
      m_nc.dac_enable = false;
      m_control.nc_enabled = false;
    }
    else
    {
      m_nc.dac_enable = true;
    }
  }

  void audio::write_reg_nr43 (std::uint8_t value)
  {
    m_nc.lfsr.state = value;

    float clock_frequency = 0.0f;
    if (m_nc.lfsr.divider == 0)
    {
      clock_frequency = 262144.0f / (0.5f * std::pow(2, m_nc.lfsr.clock_shift));
    }
    else
    {
      clock_frequency = 262144.0f / (m_nc.lfsr.divider * std::pow(2, m_nc.lfsr.clock_shift));
    }

    m_nc.clock_frequency = (std::uint64_t) (4194304.0f / clock_frequency);
  }

  void audio::write_reg_nr44 (std::uint8_t value)
  {
    m_nc.phc.state = value;
    
    if ((m_nc.phc.state & 0b10000000) == 1)
    {
      m_nc.length_timer = m_nc.ltc.initial_length_timer;
      m_nc.current_volume = m_nc.vec.initial_volume;
      m_nc.lfsr_state = 0;
      m_control.nc_enabled = m_nc.dac_enable;
    }
  }

  /* Private Methods - Ticking Methods ************************************************************/

  void audio::tick_length_timers ()
  {
    if (m_control.pc1_enabled && m_pc1.phc.length_enable)
    {
      if (++m_pc1.length_timer == 0b111111)
        m_control.pc1_enabled = false;
    }

    if (m_control.pc2_enabled && m_pc2.phc.length_enable)
    {
      if (++m_pc2.length_timer == 0b111111)
        m_control.pc2_enabled = false;
    }

    if (m_control.wc_enabled && m_wc.phc.length_enable)
    {
      if (++m_wc.length_timer == 0b11111111)
        m_control.wc_enabled = false;
    }

    if (m_control.nc_enabled && m_nc.phc.length_enable)
    {
      if (++m_nc.length_timer == 0b111111)
        m_control.nc_enabled = false;
    }
  }

  void audio::tick_frequency_sweep ()
  {
    if (m_control.pc1_enabled)
    {
      std::uint16_t period_delta = m_pc1.current_period / std::pow(2, m_pc1.psc.step);
      if (
        m_pc1.psc.direction == pulse_sweep_direction::psd_increase &&
        m_pc1.current_period + period_delta > 0x7FF
      ) {
        m_control.pc1_enabled = false;
        return;
      }

      if (++m_pc1.frequency_sweep_ticks >= m_pc1.psc.pace)
      {
        if (m_pc1.psc.pace > 0)
        {
          if (m_pc1.psc.direction == pulse_sweep_direction::psd_increase)
            { m_pc1.current_period += period_delta; }
          else
            { m_pc1.current_period -= period_delta; }
        }

        m_pc1.frequency_sweep_ticks = 0;
      }
    }
  }

  void audio::tick_envelope_sweep ()
  {
    if (m_control.pc1_enabled && m_pc1.vec.sweep_pace > 0)
    {
      if (++m_pc1.envelope_sweep_ticks >= m_pc1.vec.sweep_pace)
      {
        if (
          m_pc1.vec.direction == audio_envelope_direction::aed_increase &&
          m_pc1.current_volume < 0xF
        ) { m_pc1.current_volume++; }
        else if (
          m_pc1.vec.direction == audio_envelope_direction::aed_decrease &&
          m_pc1.current_volume > 0x0
        ) { m_pc1.current_volume--; }

        m_pc1.envelope_sweep_ticks = 0;
      }
    }

    if (m_control.pc2_enabled && m_pc2.vec.sweep_pace > 0)
    {
      if (++m_pc2.envelope_sweep_ticks >= m_pc2.vec.sweep_pace)
      {
        if (
          m_pc2.vec.direction == audio_envelope_direction::aed_increase &&
          m_pc2.current_volume < 0xF
        ) { m_pc2.current_volume++; }
        else if (
          m_pc2.vec.direction == audio_envelope_direction::aed_decrease &&
          m_pc2.current_volume > 0x0
        ) { m_pc2.current_volume--; }

        m_pc2.envelope_sweep_ticks = 0;
      }
    }

    if (m_control.nc_enabled && m_nc.vec.sweep_pace > 0)
    {
      if (++m_nc.envelope_sweep_ticks >= m_nc.vec.sweep_pace)
      {
        if (
          m_nc.vec.direction == audio_envelope_direction::aed_increase &&
          m_nc.current_volume < 0xF
        ) { m_nc.current_volume++; }
        else if (
          m_nc.vec.direction == audio_envelope_direction::aed_decrease &&
          m_nc.current_volume > 0x0
        ) { m_nc.current_volume--; }

        m_nc.envelope_sweep_ticks = 0;
      }
    }
  }

  void audio::tick_wave_period_divider ()
  {
    if (m_control.wc_enabled && ++m_wc.period_divider > 0x7FF)
    {
      m_wc.period_divider = m_wc.current_period;
      m_wc.sample_index = (m_wc.sample_index + 1) % (wave_ram_nibble_size);
      m_wc.dac_input = m_wc.read_wave_ram_nibble(m_wc.sample_index);

      switch (m_wc.olc.output_level)
      {
        case wave_output_level::wol_mute:     m_wc.dac_input = 0; break;
        case wave_output_level::wol_full:     break;
        case wave_output_level::wol_half:     m_wc.dac_input = (m_wc.dac_input >> 1) & 0b111; break;
        case wave_output_level::wol_quarter:  m_wc.dac_input = (m_wc.dac_input >> 2) & 0b11;  break;
        default: break;
      }

      m_wc.dac_output = -(((float) m_wc.dac_input / 7.5f) - 1.0f);
    }
  }

  void audio::tick_pulse_period_dividers ()
  {
    if (m_control.pc1_enabled && ++m_pc1.period_divider > 0x7FF)
    {
      m_pc1.period_divider = m_pc1.current_period;
      m_pc1.wave_pointer = (m_pc1.wave_pointer + 1) % 8;
      m_pc1.dac_input = 
        (wave_duty_patterns[m_pc1.ldc.wave_duty_cycle] >> m_pc1.wave_pointer) & 1;
      m_pc1.dac_input *= m_pc1.current_volume;
      m_pc1.dac_output = -(((float) m_pc1.dac_input / 7.5f) - 1.0f);
    }

    if (m_control.pc2_enabled && ++m_pc2.period_divider > 0x7FF)
    {
      m_pc2.period_divider = m_pc2.current_period;
      m_pc2.wave_pointer = (m_pc2.wave_pointer + 1) % 8;
      m_pc2.dac_input = 
        (wave_duty_patterns[m_pc2.ldc.wave_duty_cycle] >> m_pc2.wave_pointer) & 1;
      m_pc2.dac_input *= m_pc2.current_volume;
      m_pc2.dac_output = -(((float) m_pc2.dac_input / 7.5f) - 1.0f);
    }
  }

  void audio::tick_noise_divider ()
  {
    std::uint8_t
      bit0 = (m_nc.lfsr_state & 0b1),
      bit1 = ((m_nc.lfsr_state >> 1) & 0b1),
      bit15 = (bit0 == bit1) ? 1 : 0;

    m_nc.lfsr_state |= (bit15 << 15);
    if (m_nc.lfsr.width) { m_nc.lfsr_state |= (bit15 << 7); }

    m_nc.lfsr_state >>= 1;
    m_nc.dac_input = (m_nc.lfsr_state & 0b1) * m_nc.current_volume;
    m_nc.dac_output = -(((float) m_nc.dac_input / 7.5f) - 1.0f);
  }

}

