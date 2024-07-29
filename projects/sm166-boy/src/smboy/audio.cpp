/** @file smboy/audio.hpp */

#include <smboy/emulator.hpp>
#include <smboy/audio.hpp>

namespace smboy
{

  /** Public Methods - Initialization *************************************************************/
  
  void audio::initialize (emulator* _emulator)
  {
    m_emulator = _emulator;
    m_control.state = 0xF1;
    m_panning.state = 0xF3;
    m_volume.state  = 0x77;
  }
  
  /** Public Methods - Tick ***********************************************************************/
  
  void audio::tick (bool needs_update)
  {
    if (needs_update == true)
    {
    
    }
  }

  /** Public Methods - Register Writes With Side Effects ******************************************/
  
  void audio::write_reg_nr12 (std::uint8_t value)
  {
    m_pc1.vec.state = value;
    if ((m_pc1.vec.state & 0b11111000) == 0)
    {
    
    }
  }
  
  void audio::write_reg_nr14 (std::uint8_t value)
  {
    m_pc1.phc.state = value;
    
    if ((m_pc1.phc.state & 0b10000000) == 1)
    {
    
    }
  }
  
  void audio::write_reg_nr22 (std::uint8_t value)
  {
    m_pc2.vec.state = value;
    if ((m_pc2.vec.state & 0b11111000) == 0)
    {
    
    }
  }
  
  void audio::write_reg_nr24 (std::uint8_t value)
  {
    m_pc2.phc.state = value;
    
    if ((m_pc2.phc.state & 0b10000000) == 1)
    {
    
    }
  }
  
  void audio::write_reg_nr30 (std::uint8_t value)
  {
    m_wc.dac = value;
    if ((m_wc.dac & 0b10000000) == 0)
    {
    
    }
  }

  void audio::write_reg_nr34 (std::uint8_t value)
  {
    m_wc.phc.state = value;
    
    if ((m_wc.phc.state & 0b10000000) == 1)
    {
    
    }
  }

}

