;
; @file counter_logic.asm
;

def increment:  inc b0
                ret nz
                inc high_c
                ret n

def main:       ld b0, 0
                st high_c, b0

def main_loop:  call n, increment
                jmp nz, main_loop
                stop