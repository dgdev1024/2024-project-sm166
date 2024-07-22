/** @file smboy/renderer.hpp */

#pragma once

#include <smboy/common.hpp>

namespace smboy
{

  class emulator;
  
  /** Display Control and Status Registers ********************************************************/
  
  /**
   * @brief The @a `display_mode` enum enumerates the four display modes under which the renderer
   *        operates.
   */
  enum display_mode
  {
    dm_horizontal_blank = 0b00,
    dm_vertical_blank   = 0b01,
    dm_object_scan      = 0b10,
    dm_drawing_pixels   = 0b11
  };
  
  /**
   * @brief The @a `lcd_control` union describes the current state of the renderer's display control
   *        register (`LCDC`), which describes the renderer's current configuration.
   */
  union lcd_control
  {
    struct
    {
      // Are the background and window layers allowed to have priority over the object layer?
      std::uint8_t  bgw_priority      : 1;
      
      // Is the objects layer currently enabled?
      std::uint8_t  obj_enable        : 1;
      
      // What height should the renderer use for processing objects?
      // - If this is set, then objects should be 16 pixels tall.
      // - If this is clear, then objects should be 8 pixels tall.
      std::uint8_t  tall_objects      : 1;
      
      // Which 32x32 tilemap should be used to select tiles for rendering the background layer?
      // - If this is set, then the tilemap starting at VRAM address $1C00 should be used.
      // - If this is clear, then the tilemap starting at VRAM address $1800 should be used.
      std::uint8_t  bg_tilemap        : 1;
      
      // Indicates the addressing mode by which tiles are selected for rendering the background and
      // window layers.
      // - If this is set, then indices 0 - 127 will select tiles starting at VRAM address $0000.
      // - If this is clear, then indices 0 - 127 will select tiles starting at VRAM address $1000.
      //
      // In either case, indices 128 - 255 will select tiles starting at VRAM address $0800.
      std::uint8_t  bgw_address_mode  : 1;
      
      // Is the window layer currently enabled?
      std::uint8_t  win_enable        : 1;
      
      // Which 32x32 tilemap should be used to select tiles for rendering the window layer?
      // - If this is set, then the tilemap starting at VRAM address $1C00 should be used.
      // - If this is clear, then the tilemap starting at VRAM address $1800 should be used.
      std::uint8_t  win_tilemap       : 1;
      
      // Indicates whether the renderer is currently enabled.
      std::uint8_t  master_enable     : 1;
    };
    
    std::uint8_t state;
  };
  
  /**
   * @brief The @a `lcd_status` union describes the current state of the renderer's display status
   *        register (`STAT`), which describes the current state of the renderer.
   */
  union lcd_status
  {
    struct
    {
      // The renderer's current display mode. Read-only.
      std::uint8_t  mode                : 2;
      
      // Indicates whether the number of the scanline currently being rendered matches the value of
      // the renderer's line compare register. Read-only to the processor.
      std::uint8_t  line_compare        : 1;
      
      // Indicates whether an LCD stat interrupt should be requested when the renderer's display
      // mode changes to horizontal blank (`hblank`).
      std::uint8_t  hblank_stat         : 1;
      
      // Indicates whether an LCD stat interrupt should be requested when the renderer's display
      // mode changes to vertical blank (`vblank`).
      std::uint8_t  vblank_stat         : 1;
      
      // Indicates whether an LCD stat interrupt should be requested when the renderer's display
      // mode changes to object scan.
      std::uint8_t  oam_stat            : 1;
      
      // Indicates whether an LCD stat interrupt should be requested when the renderer begins
      // drawing the scanline indicated by the line compare register.
      std::uint8_t  lyc_stat            : 1;
      std::uint8_t                      : 1;
    };
    
    std::uint8_t state;
  };
  
  /** Tile Attributes *****************************************************************************/
  
  /**
   * @brief The @a `tile_attributes` union contains the flags used to describe a tile.
   */
  union tile_attributes
  {
    struct
    {
      // This is the number of the color palette used to render the tile (0 - 7).
      std::uint8_t  palette_number      : 3; 
      
      // This is the number of the VRAM bank containing the tile data table from which the
      // tile should be selected (0 - 2).
      std::uint8_t  bank_number         : 1; 
      
      std::uint8_t                      : 1;
      
      // Should the tile be flipped horizontally when rendered?
      std::uint8_t  x_flip              : 1;
      
      // Should the tile be flipped vertically when rendered?
      //
      // Note: In the case of objects, if tall objects mode is enabled, this setting will flip
      // the entire object, not its individual tiles.
      std::uint8_t  y_flip              : 1;
      
      // Should the background and window layers have priority over any objects residing over a
      // particular pixel on this tile?
      std::uint8_t  bgw_priority        : 1;
    };
    
    std::uint8_t state;
  };
  
  /** Object Attribute Memory Entry ***************************************************************/
  
  /**
   * @brief The @a `object` struct describes an entry in the renderer's object attribute memory
   *        (OAM).
   */
  struct object
  {
    std::uint8_t    y_position;   // The object's Y position, plus 16.
    std::uint8_t    x_position;   // The object's X position, plus 8.
    
    // The number of the tile used to represent this object.
    //
    // Note: If tall objects mode is enabled, then two tiles will be needed to represent the object.
    // In this case, the number of the top tile will be this value with the low bit cleared, and the
    // number of the bottom tile will be this value with the low bit set.
    std::uint8_t    tile_number;  
    
    // Contains the object's tile attributes.
    tile_attributes attributes;
  };
  
  /** Pixel Fetcher *******************************************************************************/
  
  // The pixel FIFO is a queue of color values which the renderer pushes into the screen buffer
  // to be displayed.
  using pixel_fifo = std::queue<std::uint32_t>;
  
  
  /**
   * @brief The @a `pixel_fetch_mode` enum enumerates the five states of the renderer's pixel
   *        fetching mechanism.
   */
  enum pixel_fetch_mode
  {
    pfm_tile_number,
    pfm_tile_data_low,
    pfm_tile_data_high,
    pfm_sleep,
    pfm_push
  };
  
  /**
   * @brief The @a `pixel_fetcher` struct describes the renderer's pixel-fetching mechanism.
   */
  struct pixel_fetcher
  {
    // The pixel FIFO is a queue of color values to be pushed into the screen buffer.
    pixel_fifo        fifo;
    
    pixel_fetch_mode  mode;     // The pixel fetcher's current mode of operation.
    std::uint8_t      line_x;   // The current X coordinate on the current scanline.
    std::uint8_t      pushed_x; // The X coordinate of the last pixel pushed to the screen.
    std::uint8_t      fetch_x;  // The X coordinate of the pixel currently being fetched.
    std::uint8_t      map_y;    // The current Y coordinate in the 256x256 pixel tilemaps.
    std::uint8_t      map_x;    // The current X coordinate in the 256x256 pixel tilemaps.
    std::uint8_t      tile_y;   // The starting offset address of the tile data being fetched.
    std::uint8_t      fifo_x;   // The FIFO's current X position.
    
    // An array of three bytes used for loading background/window tilemap numbers and tile data.
    // - Element `0` is the tilemap number of the tile to load.
    // - Elements `1` and `2` are the low and high bytes of the loaded tile data.
    // - Element `3` is the byte containing the tile's attribute flags.
    std::uint8_t      bgw_fetch_data[4];
    
    // An array of six bytes used for loading object tile data.
    // - The odd-numbered indices point to the low bytes of the loaded object tile data.
    // - The even-numbered indices point to the high bytes of the loaded object tile data.
    std::uint8_t      obj_fetch_data[6];
    
    // The number of OAM entries fetched during the processing of the current pixel.
    std::uint8_t      fetched_obj_count;
    
    // An array of indices which point to objects in OAM which have been fetched during the
    // processing of the current pixel.
    std::uint8_t      fetched_obj_indices[3];
  };
  
  /** Renderer Class ******************************************************************************/

  /**
   * @brief The @a `renderer` class is the SM166-Boy's picture processing unit (PPU), responsible
   *        for drawing pixels on screen.
   */
  class renderer
  {
  
  public:
  
    renderer ();

  public: /** Public Methods **********************************************************************/

    void initialize (emulator* _emulator);
    void tick (const std::uint64_t& cycle_count);

  public: /* Memory Storage Accesses **************************************************************/
    
    std::uint8_t read_vram (std::uint32_t address) const;
    std::uint8_t read_oam (std::uint32_t address) const;

    void write_vram (std::uint32_t address, std::uint8_t value);
    void write_oam (std::uint32_t address, std::uint8_t value);

  public: /** Hardware Register Accesses **********************************************************/
  
    inline std::uint8_t read_reg_lcdc   () const  { return m_control.state; }
    inline std::uint8_t read_reg_stat   () const  { return m_status.state; }
    inline std::uint8_t read_reg_scy    () const  { return m_scroll_y; }
    inline std::uint8_t read_reg_scx    () const  { return m_scroll_x; }
    inline std::uint8_t read_reg_ly     () const  { return m_line; }
    inline std::uint8_t read_reg_lyc    () const  { return m_line_compare; }
    inline std::uint8_t read_reg_dma4   () const  { return (m_dma_source & 0xFF); }
    inline std::uint8_t read_reg_wy     () const  { return m_window_y; }
    inline std::uint8_t read_reg_wx     () const  { return m_window_x; }
    inline std::uint8_t read_reg_vbk    () const  { return m_vram_bank; }
    inline std::uint8_t read_reg_bcps   () const  { return m_bg_pal_spec; }
    inline std::uint8_t read_reg_obps   () const  { return m_obj_pal_spec; }
    inline std::uint8_t read_reg_opri   () const  { return m_priority_mode; }
    
    inline void write_reg_lcdc  (std::uint8_t value)  { m_control.state = value; }
    inline void write_reg_stat  (std::uint8_t value)  { m_status.state |= (value & 0b11111000); }
    inline void write_reg_scy   (std::uint8_t value)  { m_scroll_y = value; }
    inline void write_reg_scx   (std::uint8_t value)  { m_scroll_x = value; }
    inline void write_reg_lyc   (std::uint8_t value)  { m_line_compare = value; }
    inline void write_reg_dma1  (std::uint32_t value) { m_dma_source |= ((value & 0xFF) << 24); }
    inline void write_reg_dma2  (std::uint32_t value) { m_dma_source |= ((value & 0xFF) << 16); }
    inline void write_reg_dma3  (std::uint32_t value) { m_dma_source |= ((value & 0xFF) <<  8); }
    inline void write_reg_wy    (std::uint8_t value)  { m_window_y = value; }
    inline void write_reg_wx    (std::uint8_t value)  { m_window_x = value; }
    inline void write_reg_bcps  (std::uint8_t value)  { m_bg_pal_spec = value; }
    inline void write_reg_obps  (std::uint8_t value)  { m_obj_pal_spec = value; }
    inline void write_reg_opri  (std::uint8_t value)  { m_priority_mode = value; }
    
  public: /** Non-Inline Hardware Register Accesses ***********************************************/
    
    std::uint8_t read_reg_bcpd () const;
    std::uint8_t read_reg_obpd () const;

    void write_reg_dma4 ();
    void write_reg_vbk (std::uint8_t value);
    void write_reg_bcpd (std::uint8_t value);
    void write_reg_obpd (std::uint8_t value);

  private: /** Renderer State Machine *************************************************************/

    void tick_horizontal_blank ();
    void tick_vertical_blank ();
    void tick_object_scan ();
    void tick_drawing_pixels ();

  private: /* DMA Transfer Methods ****************************************************************/

    void tick_oam_dma ();

  private: /* Object Scan Methods *****************************************************************/

    void clear_line_objects ();
    void load_line_objects ();

  private: /* Pixel Pipeline Methods **************************************************************/

    void push_color_value (std::uint32_t color_value);
    std::uint32_t pop_color_value ();

    std::uint32_t get_bgw_color (std::uint8_t palette_index, std::uint8_t color_index);
    std::uint32_t get_obj_color (std::uint8_t palette_index, std::uint8_t color_index);

    std::uint32_t fetch_obj_pixel (std::uint8_t bit, std::uint8_t color_index, 
      std::uint32_t color_value, std::uint8_t bgw_priority);
    bool try_add_pixel ();
    void shift_next_pixel ();

    void load_background_tile_number ();
    void load_window_tile_number ();
    void load_object_tile_number ();
    void load_object_tile_data (std::uint8_t offset);

    void process_pipeline ();
    void reset_pipeline ();

  private: /* Helper Methods **********************************************************************/

    bool is_window_visible () const;
    void increment_line_counter ();

  private: /* Video Memory Storage ****************************************************************/

    std::vector<std::uint8_t>&  m_vram;
    std::vector<std::uint8_t>   m_vram0;
    std::vector<std::uint8_t>   m_vram1;
    std::vector<object>         m_oam;
    std::vector<std::uint8_t>   m_bg_cram;
    std::vector<std::uint8_t>   m_obj_cram;
    // std::vector<std::uint32_t>  m_screen;

  private: /* Pixel Fetcher Context ***************************************************************/

    pixel_fetcher m_fetcher;

  private: /** Hardware Registers *****************************************************************/
    
    lcd_control   m_control;
    lcd_status    m_status;
    std::uint8_t  m_scroll_y              = 0;
    std::uint8_t  m_scroll_x              = 0;
    std::uint8_t  m_line                  = 0;
    std::uint8_t  m_line_compare          = 0;
    std::uint8_t  m_window_y              = 0;
    std::uint8_t  m_window_x              = 0;
    std::uint8_t  m_vram_bank             = 0;
    std::uint8_t  m_bg_pal_spec           = 0;
    std::uint8_t  m_obj_pal_spec          = 0;
    std::uint8_t  m_priority_mode         = 0;

  private: /** Internal Values ********************************************************************/
    
    std::uint32_t m_dma_source            = 0;
    std::uint8_t  m_dma_delay             = 0;
    std::uint16_t m_line_tick             = 0;
    std::uint8_t  m_window_line           = 0;

  private: /* Object Scan Values ******************************************************************/

    std::uint8_t  m_line_object_indices[object_count];
    std::uint8_t  m_line_object_count     = 0;

  private: /** Emulator Handle ********************************************************************/
  
    emulator* m_emulator = nullptr;
    
  };

}
