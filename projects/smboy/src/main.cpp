/** @file main.cpp */

#include <stdafx.hpp>
#include <arguments.hpp>

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
    
  // Get handles to the emulator's renderer and joypad context.
  auto& renderer = emulator.get_renderer();
  auto& joypad = emulator.get_joypad();


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

    // Create a font and text to use for printing the TPS.
    sf::Font font;
    font.loadFromFile("assets/source_code_regular.ttf");

    sf::Text tpsText;
    tpsText.setFont(font);
    tpsText.setCharacterSize(20);
    tpsText.setFillColor(sf::Color::Red);

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
      tpsText.setString("FPS: " + std::to_string(emulator.get_renderer().get_fps()));

      window.clear();
      window.draw(sprite);
      window.draw(tpsText);
      window.display();
    }
    
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
