/** @file smboy/renderer.cpp */

#include <smboy/emulator.hpp>
#include <smboy/renderer.hpp>

namespace smboy
{

  renderer::renderer () :
    m_vram { m_vram0 }
  {
  
  }
  
  /** Initialization and Ticking ******************************************************************/

  void renderer::initialize (emulator* _emulator)
  {
  
    m_emulator = _emulator;

    // Initialize Memory Storage
    m_vram = m_vram0;
    // m_screen.clear();   m_screen.resize(screen_width * screen_height);

    // Initialize Background Palette Memory
    for (std::size_t i = 0; i < cram_size; i += 16)
    {
      m_bg_cram[i     ] = 0xFF; m_obj_cram[i     ] = 0xFF;
      m_bg_cram[i +  1] = 0xFF; m_obj_cram[i +  1] = 0xFF;
      m_bg_cram[i +  2] = 0xFF; m_obj_cram[i +  2] = 0xFF;
      m_bg_cram[i +  3] = 0xFF; m_obj_cram[i +  3] = 0xFF;
      m_bg_cram[i +  4] = 0xAA; m_obj_cram[i +  4] = 0xAA;
      m_bg_cram[i +  5] = 0xAA; m_obj_cram[i +  5] = 0xAA;
      m_bg_cram[i +  6] = 0xAA; m_obj_cram[i +  6] = 0xAA;
      m_bg_cram[i +  7] = 0xAA; m_obj_cram[i +  7] = 0xAA;
      m_bg_cram[i +  8] = 0x55; m_obj_cram[i +  8] = 0x55;
      m_bg_cram[i +  9] = 0x55; m_obj_cram[i +  9] = 0x55;
      m_bg_cram[i + 10] = 0x55; m_obj_cram[i + 10] = 0x55;
      m_bg_cram[i + 11] = 0x55; m_obj_cram[i + 11] = 0x55;
      m_bg_cram[i + 12] = 0x00; m_obj_cram[i + 12] = 0x00;
      m_bg_cram[i + 13] = 0x00; m_obj_cram[i + 13] = 0x00;
      m_bg_cram[i + 14] = 0x00; m_obj_cram[i + 14] = 0x00;
      m_bg_cram[i + 15] = 0x00; m_obj_cram[i + 15] = 0x00;
    }

    // Initialize Internal Values
    m_dma_source            = 0x00000000;
    m_dma_delay             = 0x00;
    m_line_tick             = 0x00;
    m_window_line           = 0x00;

    // Initialize Hardware Registers
    m_control.state         = 0x91;
    m_status.state          = 0x85;
    m_scroll_y = m_scroll_x = 0x00;
    m_line = m_line_compare = 0x00;
    m_window_y = m_window_x = 0x00;
    m_vram_bank             = 0x00;
    m_bg_pal_spec           = 0x00;
    m_obj_pal_spec          = 0x00;
    m_priority_mode         = 0x00;

    // Initialize frame time...
    m_start = std::chrono::system_clock::now();

  }

  void renderer::tick (const std::uint64_t& cycle_count)
  {
  
    // Don't bother ticking if the renderer has no parent emulator attached.
    if (m_emulator == nullptr) {
      return;
    }
    
    // Check the renderer's master enable. Don't bother ticking if it is turned off.
    if (m_control.master_enable == false) {
      return;
    }

    // Increment the current line tick, then run the state machine.
    m_line_tick++;
    switch (m_status.mode)
    {
      case display_mode::dm_horizontal_blank: tick_horizontal_blank();  break;
      case display_mode::dm_vertical_blank:   tick_vertical_blank();    break;
      case display_mode::dm_object_scan:      tick_object_scan();       break;
      case display_mode::dm_drawing_pixels:   tick_drawing_pixels();    break;
      default: break;
    }

    // On each machine cycle (every four tick cycles), run the OAM DMA transfer if it is active.
    if (cycle_count % 4 == 0) {
      tick_oam_dma();
    }

  }

  /** Memory Storage Accesses *********************************************************************/

  std::uint8_t renderer::read_vram (std::uint32_t address) const
  {
    if (
      address >= vram_size ||
      m_status.mode == display_mode::dm_drawing_pixels
    ) { 
      return 0xFF; 
    }

    return m_vram[address];
  }

  std::uint8_t renderer::read_oam (std::uint32_t address) const
  {
    if (
      address >= oam_size ||
      m_status.mode == display_mode::dm_object_scan ||
      m_status.mode == display_mode::dm_drawing_pixels
    ) { 
      return 0xFF; 
    }

    const std::uint8_t* oam_bytes = reinterpret_cast<const std::uint8_t*>(m_oam);
    return oam_bytes[address]; 
  }

  void renderer::write_vram (std::uint32_t address, std::uint8_t value)
  {
    if (
      address < vram_size &&
      m_status.mode != display_mode::dm_drawing_pixels
    ) {
      // std::cout 
      //           << "Writing 0x" << (int) value << " to address $" << address << "..."
      //           << std::endl;

      m_vram[address] = value;
    }
  }

  void renderer::write_oam (std::uint32_t address, std::uint8_t value)
  {
    if (
      address < oam_size &&
      m_status.mode != display_mode::dm_object_scan &&
      m_status.mode != display_mode::dm_drawing_pixels
    ) {
      std::uint8_t* oam_bytes = reinterpret_cast<std::uint8_t*>(m_oam);
      oam_bytes[address] = value;
    }
  }

  /** Screen Buffer Accesses **********************************************************************/

  const std::uint32_t* renderer::get_screen_buffer () const
  {
    return m_screen;
  }

  const std::uint8_t* renderer::get_screen_bytes () const
  {
    return reinterpret_cast<const std::uint8_t*>(m_screen);
  }

  /** Non-Inline Hardware Register Accesses *******************************************************/

  std::uint8_t renderer::read_reg_bcpd () const
  {
    if (m_status.mode == display_mode::dm_drawing_pixels)
    {
      return 0xFF;
    }

    return m_bg_cram[m_bg_pal_spec & 0b01111111];
  }

  std::uint8_t renderer::read_reg_obpd () const
  {
    if (m_status.mode == display_mode::dm_drawing_pixels)
    {
      return 0xFF;
    }

    return m_obj_cram[m_obj_pal_spec & 0b01111111];
  }

  void renderer::write_reg_dma4 ()
  {
    m_dma_source &= 0xFFFFFF00;
    m_dma_delay = 2;
  }

  void renderer::write_reg_vbk (std::uint8_t value)
  {
    m_vram_bank = value;

    if (sm_getbit(m_vram_bank, 0) == 0) {
      m_vram = m_vram0;
    } else {
      m_vram = m_vram1;
    }
  }

  void renderer::write_reg_bcpd (std::uint8_t value)
  {
    std::uint8_t address = (m_bg_pal_spec & 0b01111111);

    if (m_status.mode != display_mode::dm_drawing_pixels)
    {
      m_bg_cram[address] = value;
    }

    if (sm_getbit(m_bg_pal_spec, 7) != 0)
    {
      m_bg_pal_spec |= ((address + 1) & 0b01111111);
    }
  }

  void renderer::write_reg_obpd (std::uint8_t value)
  {
    std::uint8_t address = (m_obj_pal_spec & 0b01111111);

    if (m_status.mode != display_mode::dm_drawing_pixels)
    {
      m_obj_cram[address] = value;
    }

    if (sm_getbit(m_obj_pal_spec, 7) != 0)
    {
      m_obj_pal_spec |= ((address + 1) & 0b01111111);
    }
  }

  /** Renderer State Machine Methods **************************************************************/

  void renderer::tick_horizontal_blank ()
  {

    // Wait until 456 line ticks have elapsed.
    if (m_line_tick >= ticks_per_line)
    {

      // Increment the line (and, if necessary, the window line).
      increment_line_counter();

      // If all 144 visible scanlines of the current frame have been rendered, then move to vertical
      // blank mode.
      if (m_line >= screen_height)
      {
      
        // Get a pointer to the emulator's processor.
        sm::processor& processor = m_emulator->get_processor();

        // Move to vertical blank mode and request the vblank interrupt.
        m_status.mode = display_mode::dm_vertical_blank;
        processor.request_interrupt(interrupt_type::int_vblank);

        // Also request a STAT interrupt, if desired.
        if (m_status.vblank_stat == 1) {
          processor.request_interrupt(interrupt_type::int_lcd);
        }

        // Update FPS
        m_current_frame++;
        m_end = std::chrono::system_clock::now();
        std::chrono::duration<float, std::milli> frame = (m_end - m_prev);
        static constexpr float FIXED_TIMESTEP = (1000.0f / 59.7f);
        if (frame.count() < FIXED_TIMESTEP)
        {
          std::this_thread::sleep_for(
            std::chrono::duration<float, std::milli>(FIXED_TIMESTEP - frame.count())
          );
        }

        std::chrono::duration<float, std::milli> dur = (m_end - m_start);
        if (dur.count() >= 1000.0f)
        {
          m_start = m_end;
          m_fps = m_current_frame;
          m_current_frame = 0;
          std::printf("[renderer] FPS: %lu\n", m_fps);
        }

        m_prev = std::chrono::system_clock::now();
        
        // On VBlank Function
        if (m_on_vblank != nullptr)
        {
          m_on_vblank(*m_emulator);
        }

      }
      else
      {

        // However, if there are still more visible scanlines to render this frame, then move back
        // to object scan mode.
        m_status.mode = display_mode::dm_object_scan;
        if (m_status.oam_stat == 1) {
          m_emulator->get_processor().request_interrupt(interrupt_type::int_lcd);
        }

      }

      // Reset the line tick counter, in any event.
      m_line_tick = 0;

    }

  }

  void renderer::tick_vertical_blank ()
  {

    // Wait until 456 line ticks have elapsed.
    if (m_line_tick >= ticks_per_line)
    {

      // Increment the current line.
      increment_line_counter();

      // Vertical blank lasts for ten scanlines worth of dots (456 dots per line, so 4,560 dots
      // all told). Once that period has elapsed, then move back to object scan mode and begin
      // processing the next frame.
      if (m_line >= lines_per_frame)
      {

        m_status.mode = display_mode::dm_object_scan;
        if (m_status.oam_stat == 1) {
          m_emulator->get_processor().request_interrupt(interrupt_type::int_lcd);
        }

        // Reset the line and window line counters.
        m_line = 0;
        m_window_line = 0;

      }

      // Reset the line ticks.
      m_line_tick = 0;

    }

  }

  void renderer::tick_object_scan ()
  {
    // If 80 ticks have elapsed in the current line, then move to the drawing pixels state.
    if (m_line_tick >= 80)
    {
      m_status.mode = display_mode::dm_drawing_pixels;
      m_fetcher.mode = pixel_fetch_mode::pfm_tile_number;
      m_fetcher.line_x = 0;
      m_fetcher.fetch_x = 0;
      m_fetcher.pushed_x = 0;
      m_fetcher.fifo_x = 0;
    }

    // Scan the OAM for line objects on the first tick of this mode.
    if (m_line_tick == 1)
    {
      m_line_object_count = 0;
      load_line_objects();
    }
  }

  void renderer::tick_drawing_pixels ()
  {
    process_pipeline();

    // Once enough pixels have been pushed to the screen buffer to draw a full scanline, then move
    // to horizontal blank mode.
    if (m_fetcher.pushed_x >= screen_width)
    {

      // Reset the pixel pipeline when we're done with it.
      reset_pipeline();

      // Move to horizontal blank mode. Request a STAT interrupt here if desired.
      m_status.mode = display_mode::dm_horizontal_blank;
      if (m_status.hblank_stat == 1) {
        m_emulator->get_processor().request_interrupt(interrupt_type::int_lcd);
      }

    }
  }

  /* DMA Transfer Methods *************************************************************************/

  void renderer::tick_oam_dma ()
  {
    std::uint8_t dma_byte = (m_dma_source & 0xFF);
    if (dma_byte < 0xA0)
    {
      if (m_dma_delay > 0)
      {
        m_dma_delay--;
      }
      else
      {
        std::uint8_t* oam_bytes = reinterpret_cast<std::uint8_t*>(m_oam);
        oam_bytes[dma_byte] = m_emulator->get_bus().read_byte(m_dma_source);
        m_dma_source++;
      }
    }
  }

  /* Object Scan Methods **************************************************************************/

  void renderer::clear_line_objects ()
  {
    m_line_object_count = 0;
  }

  void renderer::load_line_objects ()
  {

    // Don't bother checking for any more objects if the maximum of ten objects were already picked
    // up on this line.
    if (m_line_object_count == objects_per_line) { return; }

    // Check the control register for the current object height.
    std::uint8_t object_height = (m_control.tall_objects == 1) ? 16 : 8;

    // The low bit of the OPRI hardware register dictates how priority among OAM objects is
    // determined.
    std::uint8_t object_priority = sm_getbit(m_priority_mode, 0);
    if (object_priority == 0)
    {

      // If the bit is cleared, then the object's priority is determined by its index in OAM.
      // The lower the index, the higher the priority.
      //
      // As such, iterate over OAM in reverse order here.
      for (std::int8_t object_index = object_count - 1; object_index >= 0; --object_index)
      {
        const object& obj = m_oam[object_index];

        // Add the object's index to the line object indices array if the following are true:
        //
        // - The object's X position is greather than zero.
        // - The current line counter, plus 16 pixels, is greater than or equal to the object's Y 
        //    position.
        // - The current line counter, plus 16 pixels, is less than the object's Y position, plus 
        //    the object height.
        if (
          obj.x_position > 0 &&
          m_line + 16 >= obj.y_position &&
          m_line + 16 <  obj.y_position + object_height
        ) {
          m_line_object_indices[m_line_object_count++] = object_index;
          if (m_line_object_count == objects_per_line) { break; }
        }
      }

    }
    else if (object_priority == 1)
    {

      // If the bit is set, then the object's priority is determined by its X position. The smaller
      // this coordinate, the higher the priority. If two line objects share the same X position,
      // then the object with the lower index in OAM has priority.
      for (std::size_t object_index = 0; object_index < object_count; ++object_index)
      {
        const object& obj = m_oam[object_index];
        
        if (
          obj.x_position > 0 &&
          m_line + 16 >= obj.y_position &&
          m_line + 16 <  obj.y_position + object_height
        ) {
          m_line_object_indices[m_line_object_count++] = object_index;

          // In this case, when a line object is inserted, then the line object indices array needs
          // to be sorted to ensure that the referenced objects' X positions are in descending
          // order.
          std::sort(
            std::begin(m_line_object_indices),
            std::begin(m_line_object_indices) + m_line_object_count,
            [&] (const std::uint8_t index_a, const std::uint8_t index_b)
            {
              const auto& object_a = m_oam[index_a];
              const auto& object_b = m_oam[index_b];

              if (object_a.x_position == object_b.x_position) {
                return index_a > index_b;
              } else {
                return object_a.x_position > object_b.x_position;
              }
            }
          );

          if (m_line_object_count == objects_per_line) { break; }
        }
      }

    }

  }

  /* Pixel Pipeline Methods ***********************************************************************/

  void renderer::push_color_value (std::uint32_t color_value)
  {
    // m_fetcher.fifo.push(color_value);
    // m_fetcher.fifo[m_fetcher.size++] = color_value;

    m_fetcher.fifo[m_fetcher.rear] = color_value;
    m_fetcher.rear = (m_fetcher.rear + 1) % 32;
    m_fetcher.size++;
  }

  // std::uint32_t renderer::pop_color_value ()
  void renderer::pop_color_value (std::uint32_t& color_value)
  {
    // std::uint32_t color_value = m_fetcher.fifo.front();
    // m_fetcher.fifo.pop();
    // return color_value;
    // return m_fetcher.fifo[--m_fetcher.size];

    color_value = m_fetcher.fifo[m_fetcher.front];
    m_fetcher.front = (m_fetcher.front + 1) % 32;
    m_fetcher.size--;
  }

  std::uint32_t renderer::get_bgw_color (std::uint8_t palette_index, std::uint8_t color_index)
  {

    // Ensure that the palette and color indices are correct.
    palette_index = (palette_index  % 8);
    color_index   = (color_index    % 4);

    // Calculate the starting index in color RAM of the color we wish to obtain.
    std::uint8_t start_index = (palette_index * bytes_per_palette) + (color_index * 4);

    // Retrieve the color components from color RAM. Tney are laid out in the following order:
    //  - Red, Green, Blue, Alpha
    //
    // Create the color value by bitwise OR'ing the above bytes. Return the result.
    return        (m_bg_cram[start_index]      << 24) |
                  (m_bg_cram[start_index + 1]  << 16) |
                  (m_bg_cram[start_index + 2]  <<  8) |
                  (m_bg_cram[start_index + 3]       );

  }

  std::uint32_t renderer::get_obj_color (std::uint8_t palette_index, std::uint8_t color_index)
  {

    // The process of retrieving an object's color works just the same as with the background
    // color, just with object CRAM.

    palette_index = (palette_index  % 8);
    color_index   = (color_index    % 4);

    std::uint8_t start_index = (palette_index * bytes_per_palette) + (color_index * 4);

    return        (m_obj_cram[start_index]      << 24) |
                  (m_obj_cram[start_index + 1]  << 16) |
                  (m_obj_cram[start_index + 2]  <<  8) |
                  (m_obj_cram[start_index + 3]       );
  }  

  std::uint32_t renderer::fetch_obj_pixel (std::uint8_t bit, std::uint8_t color_index, 
    std::uint32_t color_value, std::uint8_t bgw_priority)
  {

    // The `color_index` parameter contains the index of the color used to render a background or
    // window layer tile over which one or more of the objects fetched by the pixel fetcher may
    // reside.
    //
    // Store that old color index here, as we may still need it.
    std::uint8_t bgw_color_index = color_index;

    // Loop through the indices of the objects fetched during the processing of the current pixel.
    for (std::size_t i = 0; i < m_fetcher.fetched_obj_count; ++i)
    {

      // Get a handle to the fetched object in OAM.
      std::uint8_t obj_index = m_fetcher.fetched_obj_indices[i];
      const object& obj = m_oam[obj_index];

      // Using the object's X position in the visible screen, ensure that the fetcher has not
      // already past that point.
      std::uint8_t obj_x = (obj.x_position - 8) + (m_scroll_x % 8);
      if (obj_x + 8 < m_fetcher.fifo_x) { continue; }

      // Using the offset bettween the object's and fetcher's X positions, ensure that the pixel
      // is within the screen's bounds.
      std::int8_t offset = m_fetcher.fifo_x - obj_x;
      if (offset < 0 || offset > 7) { continue; }

      // Using the above-calculated offset and the object's `x_flip` attribute, correct the given
      // bit number.
      bit = (obj.attributes.x_flip == true) ? offset : (7 - offset);

      // Grab the proper bit from the proper low and high bytes. Bitwise OR these bits together to
      // retrieve the color index.
      std::uint8_t  low_bit     = !!(m_fetcher.obj_fetch_data[i * 2] & (1 << bit)),
                    high_bit    = !!(m_fetcher.obj_fetch_data[(i * 2) + 1] & (1 << bit));
                    color_index = (high_bit << 1) | low_bit;

      // When it comes to objects, a color index of zero indicates transparency. If the color index
      // retrieved is zero, then ignore this object.
      if (color_index == 0) { continue; }

      // Check to see if one of the following is true:
      // - The background and window layers do not have priority over this object.
      // - The background/window color index (`bgw_color_index`) is zero.
      if (
        bgw_color_index == 0 ||
        m_control.bgw_priority == 0 ||
        (
          bgw_priority == 0 &&
          obj.attributes.bgw_priority == 0
        )
      ) {

        // Read the proper color value from the object CRAM.
        color_value = get_obj_color(obj.attributes.palette_number, color_index);
        if (color_index > 0) { break; }

      }

    }

    return color_value;

  }

  bool renderer::try_add_pixel ()
  {

    // Ensure that the fetcher's FIFO is not currently full.
    if (m_fetcher.size > 8) { return false; }

    // Element 3 of the `bgw_fetch_data` array contains the background/window tile's attributes.
    tile_attributes attributes = { .state = m_fetcher.bgw_fetch_data[3] };
    
    // Offset the fetcher's X coordinate using the horizontal background scroll register to ensure
    // that this pixel appears on screen.
    int offset_x = m_fetcher.fetch_x - (8 - (m_scroll_x % 8));

    // Don't bother adding anything to the FIFO if the above-calculated X offset is outside of the
    // screen's bounds.
    if (offset_x < 0) { return true; }

    // Iterate over the eight pixels to be added to the fetcher's FIFO.
    for (std::uint8_t i = 0; i < 8; ++i)
    {

      // Determine which bit of the high and low tile data bytes needs to be added.
      std::uint8_t bit = (attributes.x_flip == false) ? (7 - i) : i;

      // Grab the proper bit from the low and high bytes. Bitwise OR these bits together to retrieve
      // the color index.
      std::uint8_t low_bit     = !!(m_fetcher.bgw_fetch_data[1] & (1 << bit));
      std::uint8_t high_bit    = !!(m_fetcher.bgw_fetch_data[2] & (1 << bit));
      std::uint8_t color_index = (high_bit << 1) | low_bit;

      // Retrieve the proper color to render the background/window pixel.
      std::uint32_t color_value = get_bgw_color(attributes.palette_number, color_index);

      // If the object layer is currently enabled and there is at least one object residing on this
      // pixel, then fetch the appropriate pixel color from the object with priority, instead.
      if (m_control.obj_enable == 1) {
        color_value = fetch_obj_pixel(bit, color_index, color_value, attributes.bgw_priority);
      }

      // Add the pixel to the FIFO.
      push_color_value(color_value);
      m_fetcher.fifo_x++;

    }

    return true;

  }

  void renderer::shift_next_pixel ()
  {

    // Only shift a pixel from the FIFO when there are more than eight pixels present therein.
    if (m_fetcher.size > 8)
    {

      // Pop the next color value from the FIFO.
      std::uint32_t color_value; pop_color_value(color_value);

      // Ensure that the FIFO's current pixel is within screen bounds.
      if (m_fetcher.line_x >= (m_scroll_x % 8))
      {
        // Determine the index in the screen buffer where the pixel needs to be emplaced.
        std::uint32_t index = m_fetcher.pushed_x + (m_line * screen_width);

        (void) color_value;
        (void) index;

        // Emplace the pixel. Advance the FIFO's pushed pixel count afterward.
        m_screen[index] = color_value;
        m_fetcher.pushed_x++;
      }

      // Move the fetcher to the next pixel on the current scanline.
      m_fetcher.line_x++;

    }

  }

  void renderer::load_background_tile_number ()
  {
    // Determine the starting address of the 32x32 tilemap being used to render the background layer.
    std::uint32_t tilemap_address = ((m_control.bg_tilemap == 1) ? 0x1C00 : 0x1800);

    // Determine the source Y position of the tile.
    std::uint8_t tile_y = m_fetcher.map_y / 8;

    // Use the fetcher's map coordinates to determine the target address offset.
    std::uint32_t target_offset   = (m_fetcher.map_x / 8) + (tile_y * 32),
                  target_address  = tilemap_address + target_offset;

    // Read the tilemap number from VRAM bank 0, and the tilemap attributes from VRAM bank 1.
    // Place the values in elements `0` and `3` of the fetch data array, respectively.
    m_fetcher.bgw_fetch_data[0] = m_vram0[target_address];
    m_fetcher.bgw_fetch_data[3] = m_vram1[target_address];

    // std::cout << std::hex 
    //           << "Tilemap Start Address: " << tilemap_address << " | "
    //           << "Tile Y: " << (int) tile_y << " | "
    //           << "Target Offset: " << target_offset << " | "
    //           << "Target Address: " << target_address << " | "
    //           << "Tile Number: " << (int) m_fetcher.bgw_fetch_data[0]
    //           << std::endl;
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
  
  }

  void renderer::load_window_tile_number ()
  {
    // Check to see if the following cases are true:
    //  - The window layer is currently enabled and visible.
    //  - The fetcher's next X position is inside the window layer's bounds.
    //  - The current line counter's position is inside the window layer's bounds.
    if (
      is_window_visible() == true &&

      m_fetcher.fetch_x + 7u >= m_window_x &&
      m_fetcher.fetch_x + 7u < (m_window_x + 14 + screen_height) &&

      m_line >= m_window_y &&
      m_line < (m_window_y + screen_width)
    ) {

      // Determine the starting address of the 32x32 tilemap being used to render the window layer.
      std::uint32_t tilemap_address = ((m_control.win_tilemap == 1) ? 0x1C00 : 0x1800);

      // Determine the source Y position of the window tile.
      std::uint8_t tile_y = m_window_line / 8;

      // Determine the target memory address to read the tilemap index at.
      std::uint32_t target_address =  tilemap_address +
                                      ((m_fetcher.fetch_x + 7 - m_window_x) / 8) +
                                      (tile_y * 32);

      // Read the tilemap number from VRAM bank 0.
      m_fetcher.bgw_fetch_data[0] = m_vram0[target_address];

    }
  }

  void renderer::load_object_tile_number ()
  {

    // Iterate over indices of all objects residing on the current scanline.
    for (std::size_t i = 0; i < m_line_object_count; ++i)
    {

      // Get the object pointed to from OAM.
      std::uint8_t obj_index = m_line_object_indices[i];
      const object& obj = m_oam[obj_index];

      // Calculate the object's scrolling offset, if any.
      std::int16_t obj_x = (obj.x_position - 8) + (m_scroll_x % 8);

      // Ensure that the object's tile data will fit into the pixel fetcher's FIFO.
      if (
        (obj_x >= m_fetcher.fetch_x && obj_x < (m_fetcher.fetch_x + 8)) ||
        ((obj_x + 8) >= m_fetcher.fetch_x && (obj_x + 8) < (m_fetcher.fetch_x + 8))
      ) {

        // If it does, then we need to add this object's scanline pixels to the FIFO.
        m_fetcher.fetched_obj_indices[m_fetcher.fetched_obj_count++] = obj_index;
        if (m_fetcher.fetched_obj_count == 3) { break; }

      }

    }

  }

  void renderer::load_object_tile_data (std::uint8_t offset)
  {

    // Get the pixel height of our objects.
    std::uint8_t object_height = (m_control.tall_objects == 1) ? 16 : 8;

    // Iterate over the object indices fetched for this pixel.
    for (std::uint8_t i = 0; i < m_fetcher.fetched_obj_count; ++i)
    {

      // Get the object from OAM.
      std::uint8_t obj_index = m_fetcher.fetched_obj_indices[i];
      const object& obj = m_oam[obj_index];

      // Retrieve the object tile's Y position in memory. Adjust according to the object's `y_flip`
      // attribute.
      std::uint8_t tile_y = ((m_line + 16) - obj.y_position) * 2;
      if (obj.attributes.y_flip == true) {
        tile_y = ((object_height * 2) - 2) - tile_y;
      }

      // Get the object's tile index - with the low bit cleared if it's a tall object.
      std::uint8_t tile_number = obj.tile_number;
      if (object_height == 16) { tile_number &= ~(1); }

      // Determine the memory address which needs to be read from, then read the tile data from the
      // current VRAM bank.
      std::uint32_t target_address =  (tile_number * 16) +
                                      tile_y +
                                      offset;
      m_fetcher.obj_fetch_data[(i * 2) + offset] = m_vram[target_address];

    }

  }

  void renderer::process_pipeline ()
  {

    // Start by calculating the absolute position of the pixel being processed.
    m_fetcher.map_y = m_line + m_scroll_y;
    m_fetcher.map_x = m_fetcher.fetch_x + m_scroll_x;

    // Get the address offset of the first byte of the tile data to be fetched.
    m_fetcher.tile_y = (m_fetcher.map_y % 8) * 2;

    // During each even-numbered line tick in the drawing pixels mode, the pixel fetcher will work
    // to keep supplying its FIFO with new pixels to draw on screen.
    if (m_line_tick % 2 == 0) 
    {
      switch (m_fetcher.mode)
      {

        case pixel_fetch_mode::pfm_tile_number: {

          // Set the fetched object count to zero.
          m_fetcher.fetched_obj_count = 0;

          // Get tile numbers for the...
          //  - ...background layer.
          //  - ...window layer, if enabled.
          //  - ...object layer, if enabled and there are objects on the current scanline.
          if (m_control.bgw_priority) { load_background_tile_number(); }
          if (m_control.bgw_priority && m_control.win_enable) { load_window_tile_number(); }
          if (m_control.obj_enable && m_line_object_count > 0) { load_object_tile_number(); }

          // Advance the fetcher's X coordinate by 8 pixels, then proceed to fetch the tile data.
          m_fetcher.fetch_x += 8;
          m_fetcher.mode = pixel_fetch_mode::pfm_tile_data_low;

        } break;

        case pixel_fetch_mode::pfm_tile_data_low: {

          // Get the number of the tile that needs to be fetched, then the target address to fetch
          // from. Adjust according to the BGW tile data area flag, if needed.
          std::uint8_t  tile_number = m_fetcher.bgw_fetch_data[0];
          std::uint32_t target_address = (tile_number * 16) + m_fetcher.tile_y;
          if (tile_number < 128 && m_control.bgw_address_mode == 0) {
            target_address += 0x1000;
          }

          // if (tile_number > 0x00)
          // {
          //   std::cout << std::hex
          //             << "Tile Number: " << (int) tile_number << " | "
          //             << "Target Address: " << target_address
          //             << std::endl;
          // }

          // Read and store the low byte of the tile from the current VRAM bank.
          m_fetcher.bgw_fetch_data[1] = m_vram[target_address];

          // Load any object tile data here.
          load_object_tile_data(0);

          // The next mode is to fetch the tile's high byte.
          m_fetcher.mode = pixel_fetch_mode::pfm_tile_data_high;

        } break;

        case pixel_fetch_mode::pfm_tile_data_high: {

          // Repeat the same process as with `pfm_tile_data_low`, except now for the high byte of
          // the tile. Offset the target address by an additional 1.
          std::uint8_t  tile_number = m_fetcher.bgw_fetch_data[0];
          std::uint32_t target_address = (tile_number * 16) + m_fetcher.tile_y + 1;
          if (tile_number < 128 && m_control.bgw_address_mode == 0) {
            target_address += 0x1000;
          }

          m_fetcher.bgw_fetch_data[2] = m_vram[target_address];
          load_object_tile_data(1);

          // Proceed to sleep for two line ticks.
          m_fetcher.mode = pixel_fetch_mode::pfm_sleep;

        } break;

        case pixel_fetch_mode::pfm_sleep: {

          // In this mode, the fetcher sleeps for two line ticks before it attempts to push more
          // pixels to the FIFO.
          m_fetcher.mode = pixel_fetch_mode::pfm_push;

        } break;

        case pixel_fetch_mode::pfm_push: {

          // In this mode, continously attempt to add a pixel to the FIFO until successful.
          if (try_add_pixel() == true) {
            m_fetcher.mode = pixel_fetch_mode::pfm_tile_number;
          }

        } break;

      }
    }

    // In any event, try to shift a pixel from the FIFO.
    shift_next_pixel();

  }

  void renderer::reset_pipeline ()
  {
    m_fetcher.size = 0;
    m_fetcher.front = 0;
    m_fetcher.rear = 0;
  }
  
  /* Helper Methods *******************************************************************************/

  bool renderer::is_window_visible () const
  {
    return  m_control.win_enable == 1 &&
            m_window_x <= 166 &&
            m_window_y < screen_height;
  }

  void renderer::increment_line_counter ()
  {

    // Check to see if the internal window line counter needs to be incremented as well.
    if (
      is_window_visible() == true &&
      m_line >= m_window_y &&
      m_line <  m_window_y + screen_height
    ) {
      m_window_line++;
    }

    // Increment the line counter. Check then to see if it matches the line compare register.
    if (++m_line == m_line_compare) {
      m_status.line_compare = 1;
      
      // Request a STAT interrupt if it's enabled.
      if (m_status.lyc_stat == 1) {
        m_emulator->get_processor().request_interrupt(interrupt_type::int_lcd);
      }
    } else {
      m_status.line_compare = 0;
    }

  }

}

