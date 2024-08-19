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

.section rom $200
  def Test:     word fint(fadd(2.0, 6.5))
                word ffrac(fadd(2.0, 6.5))
  def Main:     jmp n, [Main]
