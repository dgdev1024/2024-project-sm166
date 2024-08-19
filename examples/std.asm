;
; @file std.asm
;

; Magic Number
const global MAGIC_NUMBER         = $05316690

; Address Space
const global WRAM_START           = $80000000
const global SRAM_START           = $81000000
const global VRAM_START           = $82000000
const global VRAM_TD_START        = VRAM_START
const global VRAM_TM1_START       = (VRAM_START + $1800)
const global VRAM_TM2_START       = (VRAM_START + $1C00)
const global OAM_START            = $82004000
const global STACK_START          = $FFFD0000
const global HRAM_START           = $FFFF0000
const global IO_START             = $FFFFFF00

; Hardware Register Ports
const global HW_JOYB              = $00
const global HW_JOYD              = $01
const global HW_JOYC              = $02
const global HW_DIV               = $04
const global HW_TIMA              = $05
const global HW_TMA               = $06
const global HW_TAC               = $07
const global HW_RTS               = $08
const global HW_RTM               = $09
const global HW_RTH               = $0A
const global HW_RTDL              = $0B
const global HW_RTDH              = $0C
const global HW_RTC               = $0D
const global HW_IF                = $0F
const global HW_NR10              = $10
const global HW_NR11              = $11
const global HW_NR12              = $12
const global HW_NR13              = $13
const global HW_NR14              = $14
const global HW_NR21              = $16
const global HW_NR22              = $17
const global HW_NR23              = $18
const global HW_NR24              = $19
const global HW_NR30              = $1A
const global HW_NR31              = $1B
const global HW_NR32              = $1C
const global HW_NR33              = $1D
const global HW_NR34              = $1E
const global HW_NR41              = $20
const global HW_NR42              = $21
const global HW_NR43              = $22
const global HW_NR44              = $23
const global HW_NR50              = $24
const global HW_NR51              = $25
const global HW_NR52              = $26
const global HW_LCDC              = $40
const global HW_STAT              = $41
const global HW_SCY               = $42
const global HW_SCX               = $43
const global HW_LY                = $44
const global HW_LYC               = $45
const global HW_DMA1              = $46
const global HW_DMA2              = $47
const global HW_DMA3              = $48
const global HW_DMAS              = $49
const global HW_WY                = $4A
const global HW_WX                = $4B
const global HW_VBK               = $4F
const global HW_BCPS              = $68
const global HW_BCPD              = $69
const global HW_OBPS              = $6A
const global HW_OBPD              = $6B
const global HW_OPRI              = $6C
const global HW_IE                = $FF

; Hardware Register Values
const global HW_JOYB_START        = %10000000
const global HW_JOYB_SELECT       = %01000000
const global HW_JOYB_R            = %00100000
const global HW_JOYB_L            = %00010000
const global HW_JOYB_Y            = %00001000
const global HW_JOYB_X            = %00000100
const global HW_JOYB_B            = %00000010
const global HW_JOYB_A            = %00000001

const global HW_JOYD_UP           = %00000001
const global HW_JOYD_DOWN         = %00000010
const global HW_JOYD_LEFT         = %00000100
const global HW_JOYD_RIGHT        = %00001000

const global HW_JOYC_ON           = %00000001
const global HW_JOYC_BUTTONS_ON   = %00000010
const global HW_JOYC_DPAD_ON      = %00000100

const global HW_TAC_SLOWEST       = %00000000
const global HW_TAC_FASTEST       = %00000001
const global HW_TAC_FAST          = %00000010
const global HW_TAC_SLOW          = %00000011
const global HW_TAC_ON            = %00000100
const global HW_TAC_OFF           = %00000000

const global HW_LCDC_ON           = %10000000
const global HW_LCDC_WIN_TM       = %01000000
const global HW_LCDC_WIN_ON       = %00100000
const global HW_LCDC_BG_TD        = %00010000
const global HW_LCDC_BG_TM        = %00001000
const global HW_LCDC_OBJ_SIZE     = %00000100
const global HW_LCDC_OBJ_ON       = %00000010
const global HW_LCDC_BG_ON        = %00000001

const global HW_STAT_MODE_HBLANK  = %00000000
const global HW_STAT_MODE_VBLANK  = %00000001
const global HW_STAT_MODE_OAM     = %00000010
const global HW_STAT_MODE_DRAW    = %00000011  
const global HW_STAT_LY_COMPARE   = %00000100
const global HW_STAT_LCD_HBLANK   = %00001000
const global HW_STAT_LCD_VBLANK   = %00010000
const global HW_STAT_LCD_OAM      = %00100000
const global HW_STAT_LCD_LYC      = %01000000

const global HW_BCPS_AUTO_INC     = %10000000
const global HW_BCPS_PAL_INDEX    = %01111111
const global HW_OBPS_AUTO_INC     = %10000000
const global HW_OBPS_PAL_INDEX    = %01111111

; Interrupts
const global INT_VBLANK           = %00000001
const global INT_LCD              = %00000010
const global INT_TIMER            = %00000100
const global INT_SERIAL           = %00001000
const global INT_JOYPAD           = %00010000
const global INT_RTC              = %00100000

; Useful Constants
const global SCREEN_WIDTH         = 160
const global SCREEN_HEIGHT        = 144
const global VBLANK_THRESHOLD     = 144

; Tile Flags
const global TF_PALETTE_NUMBER    = %00000111
const global TF_BANK_NUMBER       = %00001000
const global TF_X_FLIP            = %00100000
const global TF_Y_FLIP            = %01000000
const global TF_BGW_PRIORITY      = %10000000

; Object Constants and Functions
const global OAM_SIZE             = 160
const global OBJECT_COUNT         = 40
const global OBJECT_SIZE          = 4
const global OBJECT_SCX_OFFSET    = 8
const global OBJECT_SCY_OFFSET    = 16

function GetObjectY (index)
{
  let global oam_start_address = OAM_START
  
  if (_count == 2)
  {
    let global oam_start_address = _1
  }

  [(oam_start_address + ((index % OBJECT_COUNT) * OBJECT_SIZE))]
}

function GetObjectX (index)
{
  let local oam_start_address = OAM_START
  
  if (_count == 2)
  {
    let local oam_start_address = _1
  }

  [(oam_start_address + ((index % OBJECT_COUNT) * OBJECT_SIZE) + 1)]
}

function GetObjectTileIndex (index)
{
  let local oam_start_address = OAM_START
  
  if (_count == 2)
  {
    let local oam_start_address = _1
  }

  [(oam_start_address + ((index % OBJECT_COUNT) * OBJECT_SIZE) + 2)]
}

function GetObjectTileFlags (index)
{
  let local oam_start_address = OAM_START
  
  if (_count == 2)
  {
    let local oam_start_address = _1
  }

  [(oam_start_address + ((index % OBJECT_COUNT) * OBJECT_SIZE) + 3)]
}

