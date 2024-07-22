/** @file main.cpp */

#include <stdafx.hpp>
#include <arguments.hpp>

void run_emulation_thread ()
{
  auto& emulator = smboy::emulator::get_instance();
  while (emulator.is_running() == true)
  {
    if (emulator.step() == false) { break; }
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

  // Start the emulation thread.
  std::thread emulation_thread { run_emulation_thread };

  if (smboy::arguments::has("headless", 'h') == false)
  {
    // Create the application window and target texture.
    sf::RenderWindow window { { 800, 600 }, emulator.get_program().get_title() };

    // Loop as long as the window is open and the emulator is running.
    while (window.isOpen() && emulator.is_running())
    {
      sf::Event ev;
      while (window.pollEvent(ev))
      {
        if (ev.type == sf::Event::Closed)
          window.close();
      }

      window.clear();
      window.display();
    }
  }
  else
  {
    while (emulator.is_running() == true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
  
  // Wait for the thread to finish before exiting.
  emulation_thread.join();

  return 0;
}
