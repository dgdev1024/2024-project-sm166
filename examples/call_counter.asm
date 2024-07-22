;
; @file examples/call_counter.asm
;


const start_point = $00

function install_magic_number ()
{
  const magic_number = _0
  .section rom $100   long magic_number
}

function install_header (name, author)
{
  install_magic_number($05316690)
  .section rom $120   byte name
  .section rom $140   byte author
}

function stop_if_done ()
{
  jmp nz, _0
  stop
}

.size $240

install_header("Counter With Fn", "Dennis W. Griffin");

.section ram $100
    def high_c:     byte 1

.section rom $210
    def increment:  inc b0
                    ret nz
                    inc high_c
                    ret n

    def main:       ld b0, start_point
                    st high_c, b0

    def main_loop:  call n, increment
                    stop_if_done(main_loop)

.section rom $200
    def start:      jmp n, main