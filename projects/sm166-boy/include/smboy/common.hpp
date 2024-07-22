/** @file smboy/common.hpp */

#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <filesystem>
#include <memory>
#include <chrono>
#include <thread>
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
  address_space(vram,  0x82000000, 0x82002000);           // Video RAM - 8 KB, 2 Banks
  address_space(oam,   0x82004000, 0x820040A0);           // Object Attribute Memory - 160 Bytes
  address_space(stack, 0xFFFD0000, 0xFFFE0000);           // Stack Memory - 64 KB
  address_space(hram,  0xFFFE0000, 0xFFFF0000);           // High RAM - 64 KB
  address_space(io,    0xFFFFFF00, 0xFFFFFFFF);           // IO Registers - 256 Bytes

  #undef address_space
  
  constexpr std::uint32_t screen_width = 160;
  constexpr std::uint32_t screen_height = 144;
  constexpr std::uint32_t object_count = 40;
  constexpr std::uint32_t ticks_per_line = 456;
  constexpr std::uint32_t lines_per_frame = 154;
  constexpr std::uint32_t objects_per_line = 10;
  constexpr std::uint32_t bytes_per_palette = 8;
  constexpr std::uint32_t cram_size = 128;
  
  enum interrupt_type
  {
    int_vblank,
    int_lcd,
    int_timer,
    int_serial,
    int_joypad,
    int_realtime
  };

  using byte_buffer = std::vector<std::uint8_t>;

}
