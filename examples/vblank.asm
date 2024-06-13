;
; @file vblank.asm
;

.size $300

.section rom $100   long $05316690                ; Magic Number
.section rom $120   byte "VBlank Counter"         ; Title
.section rom $140   byte "Dennis W. Griffin"      ; Author

; RAM - Counter Bytes
.section ram $100
  def counter: byte 1

; Counter Subroutine
.section rom $240
  def count:
    inc counter
    ld b0, counter
    cmp 200
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

    ; Set the counter bytes in RAM.
    xor b0
    st counter, b0

    ; Enable the vertical blank interrupt, then enable the interrupt master.
    ld b0, %00000001
    shb [$FF]
    ei

  def loop: jmp n, loop
