;
; @file timer.asm
;

.size $300

.section rom $100   long $05316690                ; Magic Number
.section rom $120   byte "Timer-Powered Counter"  ; Title
.section rom $140   byte "Dennis W. Griffin"      ; Author

; RAM - Counter Bytes
.section ram $100
  def cb1: byte 1
  def cb2: byte 1

; Counting Subroutine
.section rom $240
  def count:
    inc cb1
    ret nz
    inc cb2
    ret nz
    stop

; Timer Interrupt
.section rom $A0
  def int_timer:
    call n, count
    reti

; Main Function
.section rom $200
  def main:

    ; Set the counter bytes in RAM.
    xor b0
    st cb1, b0
    st cb2, b0

    ; Enable the timer and set its speed to slowest.
    ld b0, %00000101
    shb [$07]

    ; Enable the timer interrupt, then enable the interrupt master.
    ld b0, %00000100
    shb [$FF]
    ei

  def main_loop:

    ; Loop here and let the timer's interrupt handle the counting logic.
    jmp n, main_loop
