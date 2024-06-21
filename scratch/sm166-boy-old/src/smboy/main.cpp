/** @file smboy/main.cpp */

#include <smboy/arguments.hpp>
#include <smboy/emulator.hpp>

int main (int argc, char** argv)
{
  if (smboy::arguments::parse(argc, argv) == false) {
    return 1;
  }

  auto program_file = smboy::arguments::get("program-file", 'p');
  if (program_file.empty()) {
    std::cerr << "Missing program file argument (--program-file, -p)." << std::endl;
    return 1;
  }
  
  smboy::emulator emulator { program_file };
  if (emulator.start() == false) { return 1; }
  
  return 0;
}
