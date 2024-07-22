;
; @file examples/call_counter.asm
;

.size $240

.section rom $100   long $05316690
.section rom $120   byte "Counter With CALL"
.section rom $140   byte "Dennis W. Griffin"

.section ram $100
    def high_c:     byte 1

.section rom $210
    include "../examples/counter_logic.asm"

.section rom $200
    def start:      jmp n, main