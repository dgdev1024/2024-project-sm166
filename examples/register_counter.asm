;
; @file examples/register_counter.asm
;

.size $240

.section rom $100   long $05316690
.section rom $120   byte "Register Counter"
.section rom $140   byte "Dennis W. Griffin"

.section rom $208
    def main:       inc b0
                    jmp nz, main
                    stop

.section rom $200
    def start:      jmp n, main
    
