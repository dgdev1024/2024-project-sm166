/** @file smboy/common.hpp */

#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <sm/common.hpp>

namespace fs = std::filesystem;

namespace smboy
{

  #define address_space(space, start, end) \
    constexpr std::uint32_t space##_start_addr = start, \
                            space##_end_addr = end, \
                            space##_size = (end - start)

  address_space(rom,   0x00000000, 0x04000000);           // Program ROM - 64 MB
  address_space(wram,  0x80000000, 0x80800000);           // Working RAM - 8 MB
  address_space(sram,  0x81000000, 0x81040000);           // Save RAM - 256 KB
  address_space(stack, 0xFFFD0000, 0xFFFE0000);           // Stack Memory - 64 KB
  address_space(hram,  0xFFFE0000, 0xFFFF0000);           // High RAM - 64 KB
  address_space(io,    0xFFFFFF00, 0xFFFFFFFF);           // IO Registers - 256 Bytes

  #undef address_space

  enum interrupt_type
  {
    int_vblank = 0,
    int_lcd,
    int_timer,
    int_serial,
    int_joypad,
    int_realtime
  };

}