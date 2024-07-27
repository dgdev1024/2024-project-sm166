;
; @file hello-world/main.asm
;

;; Standard Library and Globals ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include "../std.asm"
const global PROGRAM_NAME     = "Hello World!"
const global PROGRAM_AUTHOR   = "Dennis W. Griffin"

;; Program Header ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $100 long MAGIC_NUMBER         ; Magic Number
.section rom $120 byte PROGRAM_NAME         ; Program Name
.section rom $140 byte PROGRAM_AUTHOR       ; Program Author

;; Program Body ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $208

  def TileData:         include "./tile-data.asm"
  def EndTileData:

  def TileMap:          include "./tile-map.asm"
  def EndTileMap:

  def CopyBytes:        ld b0, [l1]
                        st [l2], b0
                        inc l1
                        inc l2
                        dec w1
                        jmp nz, [CopyBytes]
                        ret n

  def WaitForVBlank:    lhb [HW_LY]
                        cmp 144
                        jmp c, [WaitForVBlank]
                        ret n

  def ShutdownAudio:    xor b0
                        shb [HW_NR52]
                        ret n

  def ShutdownLCD:      call n, [WaitForVBlank]
                        xor b0
                        shb [HW_LCDC]
                        ret n

  def CopyTileData:     ld l1, TileData
                        ld l2, VRAM_TD_START
                        ld w1, (EndTileData - TileData)
                        call n, [CopyBytes]
                        ret n

  def CopyTileMap:      ld l1, TileMap
                        ld l2, VRAM_TM1_START
                        ld w1, (EndTileMap - TileMap)
                        call n, [CopyBytes]
                        ret n

  def EnableLCD:        ld b0, %10010001
                        shb [HW_LCDC]
                        ret n

  def Main:             call n, [ShutdownAudio]
                        call n, [ShutdownLCD]
                        call n, [CopyTileData]
                        call n, [CopyTileMap]
                        call n, [EnableLCD]

  def Loop:             jmp n, [Loop]

;; Entry Point ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $200
  def EntryPoint:       jmp n, [Main]
