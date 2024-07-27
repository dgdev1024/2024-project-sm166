;
; @file math/main.asm
;

;; Standard Library and Globals ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include "../std.asm"
const global PROGRAM_NAME     = "Math Testbed"
const global PROGRAM_AUTHOR   = "Dennis W. Griffin"

;; Program Header ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $100 long MAGIC_NUMBER         ; Magic Number
.section rom $120 byte PROGRAM_NAME         ; Program Name
.section rom $140 byte PROGRAM_AUTHOR       ; Program Author

;; Program Body ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $208
  def TestBed:          byte strlen("Dennis W. Griffin")

  def Main:             jmp n, [Main]

;; Entry Point ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.section rom $200
  def EntryPoint:       jmp n, [Main]
