/** @file main.cpp */

#include <stdafx.hpp>
#include <arguments.hpp>

class AudioStream : public sf::SoundStream
{
public:
  static constexpr std::uint32_t SAMPLE_RATE = 44100;

public:
  AudioStream ()
  {
    initialize(2, SAMPLE_RATE);
  }

public:
  void pushSample (float left, float right)
  {
    if (left == 0 && right == 0)
    {
      return;
    }

    if (m_sampleCount + 2 < SAMPLE_RATE)
    {
      if (left  < -1.0f) { left  = -1.0f; } else if (left  > 1.0f) { left  = 1.0f; }
      if (right < -1.0f) { right = -1.0f; } else if (right > 1.0f) { right = 1.0f; }

      m_samples[m_sampleCount++] = (std::uint16_t) (left  * 32767);
      m_samples[m_sampleCount++] = (std::uint16_t) (right * 32767);
    }
  }

protected:
  virtual bool onGetData (sf::SoundStream::Chunk& data) override
  {
    std::lock_guard<std::mutex> lock { m_mutex };
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    if (m_sampleCount == 0)
    {
      return false;
    }

    data.samples = m_samples;
    data.sampleCount = m_sampleCount;
    m_sampleCount = 0;

    return true;
  }

  virtual void onSeek (sf::Time) override
  {

  }

private:
  std::mutex m_mutex;
  std::size_t m_sampleCount = 0;
  std::int16_t m_samples[SAMPLE_RATE];

};

void run_emulation_thread (smboy::emulator& emu)
{
  while (emu.is_running() == true)
  {
    if (emu.step() == false) { 
      emu.stop();
      break; 
    }
  }
}

int main (int argc, char** argv)
{
  if (smboy::arguments::parse(argc, argv) == false)
  {
    return 1;
  }

  auto program_file = smboy::arguments::get("program-file", 'p');
  if (program_file.empty() == true)
  {
    std::cerr << "[smboy] Missing required argument: --program-file, -p." << std::endl;
    return 1;
  }
  
  // Initialize the emulator.
  auto& emulator = smboy::emulator::get_instance();
  emulator.initialize();
  
  // Load the program file into the emulator.
  if (emulator.get_program().load_file(program_file) == false)
  {
    return 1;
  }

  // Create the audio stream.
  AudioStream stream;
    
  // Get handles to the emulator's renderer and joypad context.
  auto& program = emulator.get_program();
  auto& renderer = emulator.get_renderer();
  auto& joypad = emulator.get_joypad();
  auto& audio = emulator.get_audio();

  // Keep a count of how many times we hit vblank.
  std::uint32_t vblank_count = 0;
  renderer.set_vblank_function([&] (smboy::emulator&)
  {
    vblank_count++;
    if (vblank_count % 500 == 0)
    {
      program.save_sram_file();
    }
  });
  

  audio.set_mix_clock(44100.0f);
  audio.set_mix_function([&] (const smboy::audio_sample& sample)
  {
    stream.pushSample(sample.left_output, sample.right_output);
  });

  if (smboy::arguments::has("headless", 'h') == false)
  {
  
    // Start the emulation thread.
    std::thread emulation_thread { run_emulation_thread, std::ref(emulator) };

    // Create the application window and target texture.
    sf::RenderWindow window { { 160 * 4, 144 * 4 }, emulator.get_program().get_title() };
    window.setFramerateLimit(60);

    // Create a texture to contain the contents of the screen buffer.
    sf::Texture target;
    target.create(smboy::screen_width, smboy::screen_height);
    target.setSmooth(true);

    // Start the audio stream.
    stream.play();

    // Loop as long as the window is open and the emulator is running.
    while (window.isOpen() && emulator.is_running())
    {
      sf::Event ev;
      while (window.pollEvent(ev))
      {
        if (ev.type == sf::Event::Closed)
        {
          emulator.stop();
          window.close();
        }

        else if (ev.type == sf::Event::KeyPressed)
        {
          switch (ev.key.code)
          {
            case sf::Keyboard::W: joypad.set_dpad(smboy::joypad_dpad::up, true); break;
            case sf::Keyboard::S: joypad.set_dpad(smboy::joypad_dpad::down, true); break;
            case sf::Keyboard::A: joypad.set_dpad(smboy::joypad_dpad::left, true); break;
            case sf::Keyboard::D: joypad.set_dpad(smboy::joypad_dpad::right, true); break;
            case sf::Keyboard::J: joypad.set_button(smboy::joypad_button::a, true); break;
            case sf::Keyboard::K: joypad.set_button(smboy::joypad_button::b, true); break;
            case sf::Keyboard::I: joypad.set_button(smboy::joypad_button::x, true); break;
            case sf::Keyboard::N: joypad.set_button(smboy::joypad_button::y, true); break;
            case sf::Keyboard::R: joypad.set_button(smboy::joypad_button::l, true); break;
            case sf::Keyboard::U: joypad.set_button(smboy::joypad_button::r, true); break;
            case sf::Keyboard::H: joypad.set_button(smboy::joypad_button::select, true); break;
            case sf::Keyboard::G: joypad.set_button(smboy::joypad_button::start, true); break;
            case sf::Keyboard::Escape:        
              emulator.stop();
              window.close();
              break;
            default: break;
          }
        }
        
        else if (ev.type == sf::Event::KeyReleased)
        {
          switch (ev.key.code)
          {
            case sf::Keyboard::W: joypad.set_dpad(smboy::joypad_dpad::up, false); break;
            case sf::Keyboard::S: joypad.set_dpad(smboy::joypad_dpad::down, false); break;
            case sf::Keyboard::A: joypad.set_dpad(smboy::joypad_dpad::left, false); break;
            case sf::Keyboard::D: joypad.set_dpad(smboy::joypad_dpad::right, false); break;
            case sf::Keyboard::J: joypad.set_button(smboy::joypad_button::a, false); break;
            case sf::Keyboard::K: joypad.set_button(smboy::joypad_button::b, false); break;
            case sf::Keyboard::I: joypad.set_button(smboy::joypad_button::x, false); break;
            case sf::Keyboard::N: joypad.set_button(smboy::joypad_button::y, false); break;
            case sf::Keyboard::R: joypad.set_button(smboy::joypad_button::l, false); break;
            case sf::Keyboard::U: joypad.set_button(smboy::joypad_button::r, false); break;
            case sf::Keyboard::H: joypad.set_button(smboy::joypad_button::select, false); break;
            case sf::Keyboard::G: joypad.set_button(smboy::joypad_button::start, false); break;
            default: break;
          }
        }
      }

      target.update(renderer.get_screen_bytes());
      sf::Sprite sprite { target };
      sprite.setScale(4, 4);

      window.clear();
      window.draw(sprite);
      window.display();
    }
    
    stream.stop();
    emulation_thread.join();
    
  }
  else
  {
    std::uint64_t cycle_count = 0;
  
    while (emulator.is_running() == true)
    {
      if (emulator.step() == false || ++cycle_count == 10000000) { 
        emulator.stop();
        break; 
      }
    } 
  }

  return 0;
}
