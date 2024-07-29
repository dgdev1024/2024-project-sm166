;
; @file math/main.asm
;

;; Standard Library and Globals ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include "../std.asm"
const global PROGRAM_NAME     = "Testbed"
const global PROGRAM_AUTHOR   = "Dennis W. Griffin"

;; Program Header ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $100 long MAGIC_NUMBER         ; Magic Number
.section rom $120 byte PROGRAM_NAME         ; Program Name
.section rom $140 byte PROGRAM_AUTHOR       ; Program Author

;; Program Body ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $208
  def Main:             ld b0, %10000000
                        shb [HW_NR52]
                        ld b0, %11111111
                        shb [HW_NR51]
                        ld b0, %01110111
                        shb [HW_NR50]
                        ld b0, %00001000
                        shb [HW_NR10]
                        ld b0, %11000000
                        shb [HW_NR11]
                        ld b0, %11110111
                        shb [HW_NR12]
                        ld b0, %01010101
                        shb [HW_NR13]
                        ld b0, %00000111
                        shb [HW_NR14]
                        ld b0, %11000000
                        shb [HW_NR21]
                        ld b0, %11110111
                        shb [HW_NR22]
                        ld b0, %00000000
                        shb [HW_NR23]
                        ld b0, %00000111
                        shb [HW_NR24]
                        ld b0, %10000111
                        shb [HW_NR14]
                        shb [HW_NR24]
  def Loop:             jmp n, [Loop]

;; Entry Point ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $200
  def EntryPoint:       jmp n, [Main]
