/** @file sm/processor.hpp */

#pragma once

#include <sm/memory.hpp>

namespace sm
{

  /**
   * @brief The @a `processor_register_type` enum enumerates the direct and indirect general-purpose
   *        registers contained in the SM166 CPU.
   */
  enum class processor_register_type
  {
    // Direct 8-Bit Registers
    b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15,

    // Indirect 16-Bit Registers
    w0, w1, w2, w3, w4, w5, w6, w7,

    // Indirect 32-Bit Registers
    l0, l1, l2, l3

  };

  /**
   * @brief The @a `processor_flag_type` enum enumerates the types of flags indicating the current
   *        state of the SM166 CPU.
   */
  enum class processor_flag_type
  {
    zero,
    negative,
    half_carry,
    carry,
    interrupt_disable,
    interrupt_enable,
    halt,
    stop,

    z = zero,
    n = negative,
    h = half_carry,
    c = carry,
    d = interrupt_disable,
    e = interrupt_enable,
    t = halt,
    s = stop
  };

  /**
   * @brief The @a `processor_condition_type` enum enumerates certain conditions which the SM166 CPU
   *        can check for before executing control transfer instructions (`JMP`, `CALL`, etc.).
   */
  enum class processor_condition_type
  {
    none,
    zero,
    no_zero,
    carry,
    no_carry,

    n = none,
    z = zero,
    nz = no_zero,
    c = carry,
    nc = no_carry
  };

  /**
   * @brief The @a `processor` class is the central component of the SM166 CPU. It is responsible
   *        for keeping track of the program counter, stack pointer and general purpose registers,
   *        as well as for executing instructions.
   */
  class processor
  {

  public:
    using ptr = std::unique_ptr<processor>;

  public:

    /**
     * @brief Initializes the SM166 CPU, setting its internal registers to their default values.
     */
    void initialize ();

    /**
     * @brief Performs the given number of clock cycles on the SM166 CPU, clocking its attached
     *        components.
     * 
     * @param cycle_count The number of clock cycles to perform.
     * 
     * @note  @a `cycle_count` is actually a number of _machine_ cycles, one of which is actually
     *        _four_ clock cycles.
     */
    void cycle (std::uint32_t cycle_count);

    /**
     * @brief Requests a CPU interrupt to be handled if allowed.
     * 
     * @param id  The ID of the interrupt to be requested (0 - 7). 
     */
    void request_interrupt (std::uint8_t id);
    
    /**
     * @brief Steps the SM166 CPU through one cycle of execution.
     * 
     * @param mem A handle to the memory management unit (MMU) which the CPU will use to execute the
     *            next instruction. 
     */
    bool step (memory& mem);

  public:

    /**
     * @brief Retrieves the value of a general purpose register of the given @a `type`.
     * 
     * @param type  The @a `processor_register_type` enumeration indicating the register to
     *              retrieve.
     *  
     * @return  The value of the requested general purpose register. 
     */
    std::uint32_t read_register (const processor_register_type& type) const;

    /**
     * @brief Checks the current state of a processor flag of the given @a `type`.
     * 
     * @param type  The @a `processor_flag_type` enumeration indicating the flag bit to check.
     *  
     * @return  @a `true` if the requested CPU flag bit is currently set; 
     *          @a `false` otherwise. 
     */
    bool check_flag (const processor_flag_type& type) const;

  public:

    /**
     * @brief Modifies the value of a general purpose register of the given @a `type`.
     * 
     * @param type  The @a `processor_register_type` enumeration indicating the register to modify.
     * @param value The requested register's new value.
     * 
     * @note  If @a `type` is one of the direct 8-bit registers, then the upper three bytes of
     *        @a `value` are ignored.
     * @note  If @a `type` is one of the indirect 16-bit registers, then the upper two bytes of
     *        @a `value` are ignored.
     */
    void write_register (const processor_register_type& type, std::uint32_t value);

    /**
     * @brief Modifies the current state of a processor flag of the given @a `type`.
     * 
     * @param type  The @a `processor_flag_type` enumeration indicating the flag bit to set.
     * @param on    Should the requested CPU flag be set or cleared?
     */
    void set_flag (const processor_flag_type& type, bool on);

  public:

    /**
     * @brief Retrieves the current value of the program counter register, which points to the next
     *        instruction to be executed.
     * 
     * @return  The current value of the program counter register. 
     */
    inline std::uint32_t get_program_counter () const
    {
      return m_program_counter;
    }

    /**
     * @brief Retrieves the current value of the stack pointer register, which contains the lower
     *        two bytes of the next address in the stack to push data.
     * 
     * @return  The current value of the stack pointer register. 
     */
    inline std::uint16_t get_stack_pointer () const
    {
      return m_stack_pointer;
    }

    /**
     * @brief Retrieves the interrupt request register (`IR`), which indicates which CPU interrupts
     *        are currently requested to be handled.
     * 
     * @return  The current state of the interrupt request register. 
     */
    inline std::uint8_t get_interrupt_request () const
    {
      return m_interrupts_requested;
    }

    /**
     * @brief Retrieves the interrupt enable register (`IE`), which indicates which CPU interrupts
     *        are currently allowed to be handled.
     * 
     * @return  The current state of the interrupt enable register. 
     */
    inline std::uint8_t get_interrupt_enable () const
    {
      return m_interrupts_enabled;
    }

    /**
     * @brief Modifies the interrupt request register (`IR`), which indicates which CPU interrupts
     *        are currently requested to be handled.
     * 
     * @param value The new state of the interrupt request register. 
     */
    inline void set_interrupt_request (std::uint8_t value)
    {
      m_interrupts_requested = value;
    }

    /**
     * @brief Modifies the interrupt enable register (`IR`), which indicates which CPU interrupts
     *        are currently allowed to be handled.
     * 
     * @param value The new state of the interrupt enable register. 
     */
    inline void set_interrupt_enable (std::uint8_t value)
    {
      m_interrupts_enabled = value;
    }

    /**
     * @brief Sets the function to be called on each of the processor's clock cycles.
     * 
     * @param cycle_function  The function to be called. 
     */
    inline void set_cycle_function (
      const std::function<void(const std::uint64_t&)>& cycle_function
    )
    {
      m_cycle_function = cycle_function;
    }

  private:

    /**
     * @brief Advances the program counter by the given number of places, then performs the same
     *        number of machine clock cycles.
     * 
     * @param count The number of program counter places and machine cycles to advance. 
     */
    void advance (std::uint32_t count);

    /**
     * @brief Checks to see if the given processor condition has been fulfilled. This should be
     *        called when executing one of the control transfer instructions.
     * 
     * @param cond  The @a `processor_condition_type` enumeration of the condition to check for.
     *  
     * @return  @a `true` if the processor condition given has been fulfilled;
     *          @a `false` otherwise.
     */
    bool check_condition (const processor_condition_type& cond) const;

    /**
     * @brief Checks to see if the CPU interrupt with the given ID is enabled, and has been
     *        requested. If so, then the appropriate interrupt handler is called.
     * 
     * @param mem A handle to the MMU which needs to be accessed.
     * @param id  The ID of the interrupt to check for (0 - 7).
     * 
     * @return  @a `true` if the interrupt is enabled and has been requested;
     *          @a `false` otherwise.
     */
    bool check_interrupt (memory& mem, std::uint8_t id);

    /**
     * @brief At the end of every frame of execution, this method checks to see if there are any
     *        CPU interrupts which have been requested and need to be handled, provided that they
     *        are enabled.
     * 
     * @param mem   A handle to the MMU which needs to be accessed. 
     */
    void handle_interrupts (memory& mem);

  // Instruction Execution Methods
  private: // 0. General Instructions

    void execute_nop ();
    void execute_stop ();
    void execute_halt ();
    void execute_di ();
    void execute_ei ();
    void execute_daa ();
    void execute_cpl ();
    void execute_ccf ();
    void execute_scf ();

  private: // 10. Data Transfer Instructions - Load Instructions

    void execute_ld_i8  (memory& mem, const processor_register_type& dest_reg);
    void execute_ld_i16 (memory& mem, const processor_register_type& dest_reg);
    void execute_ld_i32 (memory& mem, const processor_register_type& dest_reg);
    void execute_ld_a32 (memory& mem, const processor_register_type& dest_reg);
    void execute_ld_r32 (memory& mem, const processor_register_type& dest_reg,
      const processor_register_type& src_reg);
    void execute_lhb    (memory& mem);
    void execute_lhr    (memory& mem);
    void execute_lhw    (memory& mem);

  private: // 11. Data Transfer Instructions - Store Instructions

    void execute_st_a32 (memory& mem, const processor_register_type& src_reg);
    void execute_st_r32 (memory& mem, const processor_register_type& src_reg,
      const processor_register_type& addr_reg);
    void execute_shb    (memory& mem);
    void execute_shr    (memory& mem);
    void execute_shw    (memory& mem);
    void execute_ssp    (memory& mem);
    void execute_spc    (memory& mem);

  private: // 12 - 15. Data Transfer Instructions - Move Instructions

    void execute_mv (const processor_register_type& dest_reg, 
      const processor_register_type& src_reg);
    void execute_msp (const processor_register_type& dest_reg);
    void execute_mpc (const processor_register_type& dest_reg);

  private: // 16. Data Transfer Instructions - Stack Instructions

    void execute_push (memory& mem, const processor_register_type& src_reg);
    void execute_pop (memory& mem, const processor_register_type& dest_reg);

  private: // 20. Control Transfer Instructions - Jumps

    void execute_jmp_a32 (memory& mem, const processor_condition_type& cond);
    void execute_jmp_r32 (const processor_condition_type& cond,
      const processor_register_type& addr_reg);

  private: // 22. Control Transfer Instructions - Calls

    void execute_call_a32 (memory& mem, const processor_condition_type& cond);
    void execute_rst (memory& mem);
    void execute_rst0();

  private: // 23. Control Transfer Instructions - Returns

    void execute_ret (memory& mem, const processor_condition_type& cond);
    void execute_reti (memory& mem);

  private: // 30. Arithmetic Instructions - Increments

    void execute_inc_r8 (const processor_register_type& reg);
    void execute_inc_r16 (const processor_register_type& reg);
    void execute_inc_r32 (const processor_register_type& reg);
    void execute_inc_a32 (memory& mem);
    void execute_inc_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 31. Arithmetic Instructions - Decrements

    void execute_dec_r8 (const processor_register_type& reg);
    void execute_dec_r16 (const processor_register_type& reg);
    void execute_dec_r32 (const processor_register_type& reg);
    void execute_dec_a32 (memory& mem);
    void execute_dec_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 32. Arithmetic Instructions - Addition

    void execute_add_i8 (memory& mem);
    void execute_add_r8 (const processor_register_type& reg);
    void execute_add_a32 (memory& mem);
    void execute_add_ar32 (memory& mem, const processor_register_type& addr_reg);
    void execute_adc_i8 (memory& mem);
    void execute_adc_r8 (const processor_register_type& reg);
    void execute_adc_a32 (memory& mem);
    void execute_adc_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 33. Arithmetic Instructions - Subtraction

    void execute_sub_i8 (memory& mem);
    void execute_sub_r8 (const processor_register_type& reg);
    void execute_sub_a32 (memory& mem);
    void execute_sub_ar32 (memory& mem, const processor_register_type& addr_reg);
    void execute_sbc_i8 (memory& mem);
    void execute_sbc_r8 (const processor_register_type& reg);
    void execute_sbc_a32 (memory& mem);
    void execute_sbc_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 50. Logical Instructions - AND

    void execute_and_i8 (memory& mem);
    void execute_and_r8 (const processor_register_type& reg);
    void execute_and_a32 (memory& mem);
    void execute_and_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 51. Logical Instructions - OR

    void execute_or_i8 (memory& mem);
    void execute_or_r8 (const processor_register_type& reg);
    void execute_or_a32 (memory& mem);
    void execute_or_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 52. Logical Instructions - XOR

    void execute_xor_i8 (memory& mem);
    void execute_xor_r8 (const processor_register_type& reg);
    void execute_xor_a32 (memory& mem);
    void execute_xor_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 53. Logical Instructions - CMP

    void execute_cmp_i8 (memory& mem);
    void execute_cmp_r8 (const processor_register_type& reg);
    void execute_cmp_a32 (memory& mem);
    void execute_cmp_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 60. Bitwise Instructions - BIT

    void execute_bit_r8 (memory& mem, const processor_register_type& src_reg);
    void execute_bit_a32 (memory& mem);
    void execute_bit_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 61. Bitwise Instructions - SET

    void execute_set_r8 (memory& mem, const processor_register_type& src_reg);
    void execute_set_a32 (memory& mem);
    void execute_set_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 62. Bitwise Instructions - RES

    void execute_res_r8 (memory& mem, const processor_register_type& src_reg);
    void execute_res_a32 (memory& mem);
    void execute_res_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 70. Shift and Rotate Instructions - SLA

    void execute_sla_r8 (const processor_register_type& reg);
    void execute_sla_a32 (memory& mem);
    void execute_sla_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 71. Shift and Rotate Instructions - SRA

    void execute_sra_r8 (const processor_register_type& reg);
    void execute_sra_a32 (memory& mem);
    void execute_sra_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 72. Shift and Rotate Instructions - SRL

    void execute_srl_r8 (const processor_register_type& reg);
    void execute_srl_a32 (memory& mem);
    void execute_srl_ar32 (memory& mem, const processor_register_type& addr_reg);

  private: // 73. Shift and Rotate Instructions - RL

    void execute_rl_r8 (const processor_register_type& reg);
    void execute_rl_a32 (memory& mem);
    void execute_rl_ar32 (memory& mem, const processor_register_type& addr_reg);
    void execute_rla ();

  private: // 74. Shift and Rotate Instructions - RLC

    void execute_rlc_r8 (const processor_register_type& reg);
    void execute_rlc_a32 (memory& mem);
    void execute_rlc_ar32 (memory& mem, const processor_register_type& addr_reg);
    void execute_rlca ();

  private: // 73. Shift and Rotate Instructions - RR

    void execute_rr_r8 (const processor_register_type& reg);
    void execute_rr_a32 (memory& mem);
    void execute_rr_ar32 (memory& mem, const processor_register_type& addr_reg);
    void execute_rra ();

  private: // 74. Shift and Rotate Instructions - RRC

    void execute_rrc_r8 (const processor_register_type& reg);
    void execute_rrc_a32 (memory& mem);
    void execute_rrc_ar32 (memory& mem, const processor_register_type& addr_reg);
    void execute_rrca ();

  private:

    /**
     * @brief The SM166 CPU contains sixteen one-byte, general purpose registers, each of which can
     *        be accessed either directly as direct 8-bit registers, or indirectly as one of eight
     *        16-bit registers or one of four 32-bit registers.
     * 
     * @note  Byte register 0 (`b0`) is the accumulator register. It is responsible for holding the
     *        result of arithmetic instructions and some comparison instructions.
     */
    std::uint8_t m_registers[16];

    /**
     * @brief The flags register is a special-purpose 8-bit register whose bits are used to indicate
     *        the current state of the SM166 CPU. The notes below list what each bit indicates.
     * 
     * @note  Bit 7 - Zero Flag (`Z`) - If the previous instruction had a value result, this flag
     *        indicates whether or not that value result was zero.
     * @note  Bit 6 - Negative Flag (`N`) - Indicates whether or not the previous instruction
     *        involved a subtraction.
     * @note  Bit 5 - Half Carry Flag (`H`) - If the previous instruction had a value result, this
     *        flag indciates whether or not the lower four bits of that result value either
     *        overflowed (there was a carry) or underflowed (there was a borrow).
     * @note  Bit 4 - Carry Flag (`C`) - If the previous instruction had a value result, this flag
     *        indicates whether or not that result either overflowed (there was a carry) or
     *        underflowed (there was a borrow).
     * @note  Bit 3 - Interrupt Disable (`D`) - Indicates whether the CPU should handle interrupts.
     *        Set by the `DI` instruction; Clear on the next tick after the Interrupt Enable (`E`)
     *        flag is set by the `EI` instruction.
     * @note  Bit 2 - Interrupt Enable (`E`) - Indicates whether the Interrupt Disable (`D`) flag 
     *        should be cleared on the next tick. Set by the `EI` instruction; Clear on the tick 
     *        afterward.
     * @note  Bit 1 - Halt Flag (`T`) - Indicates whether the CPU is currently halted and should not
     *        be executing instructions. Set by the `HALT` instruction; Clear when an interrupt is
     *        received.
     */
    std::uint8_t m_flags = 0b00000000;

    /**
     * @brief The program counter is a 32-bit register whose value points to the opcode of the
     *        next instruction to be executed. Its value starts just after the program header and
     *        is automatically modified as instructions are executed. Certain instructions, such as
     *        `JMP` and `CALL`, can modify this register as well.
     */
    std::uint32_t m_program_counter = 0x00000200;

    /**
     * @brief The SM166 supports a 64 KB memory stack starting at a certain address. The stack
     *        pointer is a 16-bit register whose value contains the lower two bytes of the next
     *        free location in the stack. Pushing bytes to the stack causes this register to be
     *        decremented, and popping bytes from the stack causes this register to be decremented.
     */
    std::uint16_t m_stack_pointer = 0xFFFF;

    /**
     * @brief The SM166 has support for eight different interrupts which, when enabled and
     *        requested, calls a hard-coded interrupt handler routine which should be located at the
     *        very start of the 32-bit address space.
     * 
     *        The bits in this byte indicate which interrupt sources should be handled when
     *        requested, assuming the Interrupts Disabled (`D`) flag in the flags register is 
     *        currently not set.
     */
    std::uint8_t m_interrupts_enabled = 0b00000000;

    /**
     * @brief The bits in this byte indicate which CPU interrupts have been requested. If the
     *        coresponding bit in the @a `m_interrupts_enabled` byte is set and the Interrupts
     *        Disabled (`D`) flag in the flags register is not set, then the coresponding interrupt
     *        will be handled at the end of the current step of execution.
     */
    std::uint8_t m_interrupts_requested = 0b00000000;

    /**
     * @brief Indicates the number of clock cycles which have elapsed since the SM166 CPU was
     *        initialized or reset. A clock cycle occurs when, among other events, the CPU reads
     *        and writes data through the MMU when executing instructions.
     */
    std::uint64_t m_tick_cycles = 0;

    /**
     * @brief This function is called every time clock cycle elapses (which happens, among other
     *        events, when the SM166 CPU reads and writes data while executing instructions), and
     *        should be used to clock other components attached to the CPU.
     * 
     * @param tick_cycle  The number of tick cycles elapsed at the time this function is called.
     */
    std::function<void(const std::uint64_t&)> m_cycle_function = nullptr;

  };

}
