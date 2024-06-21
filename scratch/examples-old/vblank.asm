;
; @file vblank.asm
;

.size $300

.section rom $100   long $05316690                ; Magic Number
.section rom $120   byte "VBlank Counter"         ; Title
.section rom $140   byte "Dennis W. Griffin"      ; Author

; Counter Subroutine
.section rom $240
  def count:
    inc b0
    cmp 2
    ret nz
    stop

; Vertical Blank Interrupt
.section rom $80
  def int_vblank:
    call n, count
    reti

; Main Function
.section rom $200
  def main:

    ; Enable the vertical blank interrupt, then enable the interrupt master.
    ld b0, %00000001
    shb [$FF]
    ei

    ; Set the accumulator to zero.
    xor b0

  def loop: jmp n, loop
