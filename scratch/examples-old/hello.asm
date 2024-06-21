;
; @file hello.asm
;

.size $240

.section rom $100       def magic:      long $05316690
.section rom $104       def sram_size:  long $00
.section rom $120       def title:      byte "My Counter Program"
.section rom $140       def author:     byte "Dennis W. Griffin"

.section ram $100
        def my_byte_one:        byte 1
        def my_byte_two:        byte 1

.section rom $210
        def increment:          inc my_byte_one
                                ret nz
                                inc my_byte_two
                                ret n

        def main:               ld b0, 0
                                st my_byte_one, b0
                                st my_byte_two, b0
        def loop:               call n, increment
                                jmp nz, loop
                                rst 0

.section rom $200
        def start:              jmp n, main

.section rom $000
        def _rst0:              stop
