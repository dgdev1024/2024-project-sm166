;
; @file rtc.asm
;

.size $300

.section rom $100   long $05316690                ; Magic Number
.section rom $120   byte "Real Time Counter"      ; Title
.section rom $140   byte "Dennis W. Griffin"      ; Author

; RAM - Counter Bytes
.section ram $100
  def cb1: byte 1

; Counting Subroutine
.section rom $240
  def count:
    inc cb1
    lhb [$08]
    cmp 0
    ret nz
    stop

; RTC Interrupt
.section rom $D0
  def int_clock:
    call n, count
    reti

; Main Function
.section rom $200
  def main:

    ; Set the counter bytes in RAM.
    xor b0
    st cb1, b0

    ; Enable the realtime clock.
    ld b0, %00000001
    shb [$0D]

    ; Enable the realtime interrupt, then enable the interrupt master.
    ld b0, %00100000
    shb [$FF]
    ei

  def main_loop:

    ; Loop here and let the realtime clock's interrupt handle the counting logic.
    jmp n, main_loop

