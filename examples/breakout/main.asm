;
; @file breakout/main.asm
;

;; Standard Library and Globals ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include "../std.asm"
const global PROGRAM_NAME     = "Breakout"
const global PROGRAM_AUTHOR   = "Dennis W. Griffin"

const global BRICK_LEFT       = $05
const global BRICK_RIGHT      = $06
const global BLANK_TILE       = $08

;; Program Header ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $100 long MAGIC_NUMBER         ; Magic Number
.section rom $120 byte PROGRAM_NAME         ; Program Name
.section rom $140 byte PROGRAM_AUTHOR       ; Program Author

;; RAM Variables ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section ram $000
  def wBallVelocityX: byte 1
  def wBallVelocityY: byte 1

;; Program Body ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $200
  def EntryPoint:       jmp n, [Main]

  include "./tile-data.asm"
  include "./tile-map.asm"
  
  ;
  ; Subroutine to convert a pixel position into a tilemap address.
  ;
  ; Registers:
  ;   B2 = Pixel's X Position
  ;   B3 = Pixel's Y Position
  ;   L3 (B12, B13, B14, B15) = Tilemap Address
  ;   W7 (B14, B15) = Low Bytes of Tilemap Address
  ; 
  def GetTileByPixel:   mv b0, b3
                        and %11111000
                        mv b7, b0
                        ld b6, $00
                        add w3
                        add w3
                        mv b0, b2
                        srl b0
                        srl b0
                        srl b0
                        add b7
                        mv b7, b0
                        adc b6
                        sub b7
                        mv b6, b0
                        ld l3, VRAM_TM1_START
                        add w7
                        mv w7, w3
                        ret n
                 
  ;
  ; Subroutine to check to see if a tile with the given ID is a wall tile.
  ;
  ; Registers:
  ;   B0 = Tile ID
  ;
  ; Flags:
  ;   Z = Set if the tile ID is a wall.
  ;       
  def IsWallTile:       cmp $00
                        ret z
                        cmp $01
                        ret z
                        cmp $02
                        ret z
                        cmp $04
                        ret z
                        cmp $05
                        ret z
                        cmp $06
                        ret z
                        cmp $07
                        ret n

  def CopyBytes:        ld b0, [l1]
                        st [l2], b0
                        inc l1
                        inc l2
                        dec w1
                        jmp nz, [CopyBytes]
                        ret n

  def WaitVBlank:       lhb [HW_LY]
                        cmp VBLANK_THRESHOLD
                        jmp c, [WaitVBlank]
                        ret n

  def AfterVBlank:      lhb [HW_LY]
                        cmp VBLANK_THRESHOLD
                        jmp nc, [AfterVBlank]
                        ret n

  def NextVBlank:       call n, [AfterVBlank]
                        call n, [WaitVBlank]
                        ret n

  def DisableLCD:       call n, [WaitVBlank]
                        xor b0
                        shb [HW_LCDC]
                        ret n

  def CopyTileData:     ld l1, TileData
                        ld l2, (VRAM_TD_START + $1000)
                        ld w1, (EndTileData - TileData)
                        call n, [CopyBytes]
                        ret n

  def CopyTileMap:      ld l1, TileMap
                        ld l2, VRAM_TM1_START
                        ld w1, (EndTileMap - TileMap)
                        call n, [CopyBytes]
                        ret n

  def CopyObjectData:   ld l1, Paddle
                        ld l2, VRAM_TD_START
                        ld w1, (EndPaddle - Paddle)
                        call n, [CopyBytes]
                        ld l1, Ball
                        ld l2, (VRAM_TD_START + $10)
                        ld w1, (EndBall - Ball)
                        call n, [CopyBytes]
                        ret n

  def ClearOAM:         xor b0
                        ld b8, OAM_SIZE
                        ld l1, OAM_START
  def ClearOAMLoop:     st [l1], b0
                        inc l1
                        dec b8
                        jmp nz, [ClearOAMLoop]
                        ret n

  def PlaceObjects:     
  def PlacePaddle:      ld l1, OAM_START
                        ld b0, (128 + 16)
                        st [l1], b0
                        inc l1
                        ld b0, (16 + 8)
                        st [l1], b0
                        inc l1
                        xor b0
                        st [l1], b0
                        inc l1
                        st [l1], b0
                        inc l1
  
  def PlaceBall:        ld b0, (100 + 16)
                        st [l1], b0
                        inc l1
                        ld b0, (32 + 8)
                        st [l1], b0
                        inc l1
                        ld b0, 1
                        st [l1], b0
                        inc l1
                        xor b0
                        st [l1], b0
                        ret n
                        
  def InitObjects:      
  def InitBall:         ld b0, 1
                        st [wBallVelocityX], b0
                        ld b0, -1
                        st [wBallVelocityY], b0
                        ret n

  def EnableLCD:        ld b0, (HW_LCDC_ON | HW_LCDC_BG_ON | HW_LCDC_OBJ_ON)
                        shb [HW_LCDC]
                        ret n
                        
  def MoveBall:         ld b8, [wBallVelocityX]
                        ld b0, [(OAM_START + 5)]
                        add b8
                        st [(OAM_START + 5)], b0
                        ld b8, [wBallVelocityY]
                        ld b0, [(OAM_START + 4)]
                        add b8
                        st [(OAM_START + 4)], b0
                        ret n
                        
  def BreakBrickLeft:   ld b8, BLANK_TILE
                        st [l3], b8
                        inc l3
                        st [l3], b8
                        ret n
                        
  def BreakBrickRight:  ld b8, BLANK_TILE
                        st [l3], b8
                        dec l3
                        st [l3], b8
                        ret n
                        
  def CheckBrick:       ld b0, [l3]
                        cmp BRICK_LEFT
                        call z, [BreakBrickLeft]
                        cmp BRICK_RIGHT
                        call z, [BreakBrickRight]
                        ret n
                        
  def OnBounceTop:      call n, [CheckBrick]
                        ld b0, 1
                        st [wBallVelocityY], b0
                        ret n
                        
  def OnBounceRight:    call n, [CheckBrick]
                        ld b0, -1
                        st [wBallVelocityX], b0
                        ret n
                        
  def OnBounceLeft:     call n, [CheckBrick]
                        ld b0, 1
                        st [wBallVelocityX], b0
                        ret n
                        
  def OnBounceBottom:   call n, [CheckBrick]
                        ld b0, -1
                        st [wBallVelocityY], b0
                        ret n
                        
  def OnBounceBall:     ld b0, -1
                        st [wBallVelocityY], b0
                        ret n
                        
  def CheckBounceBall:  ld b8, GetObjectY(0, OAM_START)
                        ld b0, GetObjectY(1, OAM_START)
                        add 8
                        cmp b8
                        ret nz
                        
                        ld b8, GetObjectX(0, OAM_START)
                        ld b0, GetObjectX(1, OAM_START)
                        sub 8
                        cmp b8
                        ret nc
                        
                        add (8 + 16)
                        cmp b8
                        jmp nc, [OnBounceBall]
                        ret n
                        
  def CheckBounce:      ld b0, GetObjectY(1, OAM_START)
                        sub (16 + 1)
                        mv b3, b0
                        ld b0, GetObjectX(1, OAM_START)
                        sub 8
                        mv b2, b0
                        call n, [GetTileByPixel]
                        ld b0, [l3]
                        call n, [IsWallTile]
                        call z, [OnBounceTop]
                        
                        ld b0, GetObjectY(1, OAM_START)
                        sub 16
                        mv b3, b0
                        ld b0, GetObjectX(1, OAM_START)
                        mv b2, b0
                        call n, [GetTileByPixel]
                        ld b0, [l3]
                        call n, [IsWallTile]
                        call z, [OnBounceRight]
                        
                        ld b0, GetObjectY(1, OAM_START)
                        sub 16
                        mv b3, b0
                        ld b0, GetObjectX(1, OAM_START)
                        sub (8 + 1)
                        mv b2, b0
                        call n, [GetTileByPixel]
                        ld b0, [l3]
                        call n, [IsWallTile]
                        call z, [OnBounceLeft]
                        
                        ld b0, GetObjectY(1, OAM_START)
                        sub (16 - 1)
                        mv b3, b0
                        ld b0, GetObjectX(1, OAM_START)
                        sub 8
                        mv b2, b0
                        call n, [GetTileByPixel]
                        ld b0, [l3]
                        call n, [IsWallTile]
                        call z, [OnBounceBottom]
                        
                        call n, [CheckBounceBall]
                        
                        ret n
                        
  def OnLeftDown:       ld b0, [(OAM_START + 1)]
                        dec b0
                        cmp 15
                        ret z
                        st [(OAM_START + 1)], b0
                        ret n
                        
  def OnRightDown:      ld b0, [(OAM_START + 1)]
                        inc b0
                        cmp 105
                        ret z
                        st [(OAM_START + 1)], b0
                        ret n
                        
  def CheckInput:       lhb [HW_JOYD]
                        and HW_JOYD_LEFT
                        call nz, [OnLeftDown]
                        lhb [HW_JOYD]
                        and HW_JOYD_RIGHT
                        call nz, [OnRightDown]
                        ret n

  def Main:             call n, [DisableLCD]
                        call n, [CopyTileData]
                        call n, [CopyTileMap]
                        call n, [CopyObjectData]
                        call n, [ClearOAM]
                        call n, [PlaceObjects]
                        call n, [InitObjects]
                        call n, [EnableLCD]

  def Loop:             call n, [NextVBlank]
                        call n, [MoveBall]
                        call n, [CheckBounce]
                        call n, [CheckInput]
                        jmp n, [Loop]

