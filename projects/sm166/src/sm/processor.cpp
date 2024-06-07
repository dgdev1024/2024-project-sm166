/** @file sm/processor.cpp */

#include <sm/processor.hpp>

namespace sm
{

  void processor::initialize ()
  {
    for (int i = 0; i < 16; ++i) {
      m_registers[i] = 0x00;
    }

    m_flags = 0b00000000;
    m_program_counter = 0x200;
    m_stack_pointer = 0xFFFF;
    m_tick_cycles = 0;
  }

  void processor::cycle (std::uint32_t cycle_count)
  {
    if (m_cycle_function == nullptr) {
      m_tick_cycles += (cycle_count * 4);
    } else {
      for (std::uint32_t i = 0; i < (cycle_count * 4); ++i) {
        m_cycle_function(++m_tick_cycles);
      }
    }
  }

  void processor::request_interrupt (std::uint8_t id)
  {
    sm_setbit(m_interrupts_requested, (id & 0b111), true);
  }

  void processor::step (memory& mem)
  {

    // Ensure that the CPU is not currently halted before attempting to execute the next 
    // instruction.
    if (check_flag(processor_flag_type::halt) == false) {

      // Read the opcode from the bus. 
      // 
      // The SM166's instruction opcodes are two bytes, so advance the program counter by two places
      // after reading the opcode.
      std::uint16_t opcode = mem.read_word(m_program_counter);
      //std::cout << "[processor] PC: " << std::hex << m_program_counter << ", OC: " << opcode << std::endl;
      advance(2);
      
      // Examine the opcode and see which instruction needs to be executed.
      //
      // Fair warning: This next block of code is VERY long!
      switch (opcode)
      {

        // 0. General Instructions
        case 0x0000:  execute_nop();  break;
        case 0x0001:  execute_stop(); break;
        case 0x0002:  execute_halt(); break;
        case 0x0003:  execute_di();   break;
        case 0x0004:  execute_ei();   break;
        case 0x0005:  execute_daa();  break;
        case 0x0006:  execute_cpl();  break;
        case 0x0007:  execute_ccf();  break;
        case 0x0008:  execute_scf();  break;

        // 10XX. Data Transfer Instructions - Load Instructions
        case 0x1000:  execute_ld_i8 (mem, processor_register_type::b0);   break;
        case 0x1001:  execute_ld_i8 (mem, processor_register_type::b1);   break;
        case 0x1002:  execute_ld_i8 (mem, processor_register_type::b2);   break;
        case 0x1003:  execute_ld_i8 (mem, processor_register_type::b3);   break;
        case 0x1004:  execute_ld_i8 (mem, processor_register_type::b4);   break;
        case 0x1005:  execute_ld_i8 (mem, processor_register_type::b5);   break;
        case 0x1006:  execute_ld_i8 (mem, processor_register_type::b6);   break;
        case 0x1007:  execute_ld_i8 (mem, processor_register_type::b7);   break;
        case 0x1008:  execute_ld_i8 (mem, processor_register_type::b8);   break;
        case 0x1009:  execute_ld_i8 (mem, processor_register_type::b9);   break;
        case 0x100A:  execute_ld_i8 (mem, processor_register_type::b10);  break;
        case 0x100B:  execute_ld_i8 (mem, processor_register_type::b11);  break;
        case 0x100C:  execute_ld_i8 (mem, processor_register_type::b12);  break;
        case 0x100D:  execute_ld_i8 (mem, processor_register_type::b13);  break;
        case 0x100E:  execute_ld_i8 (mem, processor_register_type::b14);  break;
        case 0x100F:  execute_ld_i8 (mem, processor_register_type::b15);  break;
        case 0x1010:  execute_ld_i16(mem, processor_register_type::w0);   break;
        case 0x1011:  execute_ld_i16(mem, processor_register_type::w1);   break;
        case 0x1012:  execute_ld_i16(mem, processor_register_type::w2);   break;
        case 0x1013:  execute_ld_i16(mem, processor_register_type::w3);   break;
        case 0x1014:  execute_ld_i16(mem, processor_register_type::w4);   break;
        case 0x1015:  execute_ld_i16(mem, processor_register_type::w5);   break;
        case 0x1016:  execute_ld_i16(mem, processor_register_type::w6);   break;
        case 0x1017:  execute_ld_i16(mem, processor_register_type::w7);   break;
        case 0x1018:  execute_ld_i32(mem, processor_register_type::l0);   break;
        case 0x1019:  execute_ld_i32(mem, processor_register_type::l1);   break;
        case 0x101A:  execute_ld_i32(mem, processor_register_type::l2);   break;
        case 0x101B:  execute_ld_i32(mem, processor_register_type::l3);   break;
        case 0x1020:  execute_ld_a32(mem, processor_register_type::b0);   break;
        case 0x1021:  execute_ld_a32(mem, processor_register_type::b1);   break;
        case 0x1022:  execute_ld_a32(mem, processor_register_type::b2);   break;
        case 0x1023:  execute_ld_a32(mem, processor_register_type::b3);   break;
        case 0x1024:  execute_ld_a32(mem, processor_register_type::b4);   break;
        case 0x1025:  execute_ld_a32(mem, processor_register_type::b5);   break;
        case 0x1026:  execute_ld_a32(mem, processor_register_type::b6);   break;
        case 0x1027:  execute_ld_a32(mem, processor_register_type::b7);   break;
        case 0x1028:  execute_ld_a32(mem, processor_register_type::b8);   break;
        case 0x1029:  execute_ld_a32(mem, processor_register_type::b9);   break;
        case 0x102A:  execute_ld_a32(mem, processor_register_type::b10);  break;
        case 0x102B:  execute_ld_a32(mem, processor_register_type::b11);  break;
        case 0x102C:  execute_ld_a32(mem, processor_register_type::b12);  break;
        case 0x102D:  execute_ld_a32(mem, processor_register_type::b13);  break;
        case 0x102E:  execute_ld_a32(mem, processor_register_type::b14);  break;
        case 0x102F:  execute_ld_a32(mem, processor_register_type::b15);  break;
        case 0x1030:  execute_ld_r32(mem, processor_register_type::b0,  processor_register_type::l0); break;
        case 0x1031:  execute_ld_r32(mem, processor_register_type::b1,  processor_register_type::l0); break;
        case 0x1032:  execute_ld_r32(mem, processor_register_type::b2,  processor_register_type::l0); break;
        case 0x1033:  execute_ld_r32(mem, processor_register_type::b3,  processor_register_type::l0); break;
        case 0x1034:  execute_ld_r32(mem, processor_register_type::b4,  processor_register_type::l0); break;
        case 0x1035:  execute_ld_r32(mem, processor_register_type::b5,  processor_register_type::l0); break;
        case 0x1036:  execute_ld_r32(mem, processor_register_type::b6,  processor_register_type::l0); break;
        case 0x1037:  execute_ld_r32(mem, processor_register_type::b7,  processor_register_type::l0); break;
        case 0x1038:  execute_ld_r32(mem, processor_register_type::b8,  processor_register_type::l0); break;
        case 0x1039:  execute_ld_r32(mem, processor_register_type::b9,  processor_register_type::l0); break;
        case 0x103A:  execute_ld_r32(mem, processor_register_type::b10, processor_register_type::l0); break;
        case 0x103B:  execute_ld_r32(mem, processor_register_type::b11, processor_register_type::l0); break;
        case 0x103C:  execute_ld_r32(mem, processor_register_type::b12, processor_register_type::l0); break;
        case 0x103D:  execute_ld_r32(mem, processor_register_type::b13, processor_register_type::l0); break;
        case 0x103E:  execute_ld_r32(mem, processor_register_type::b14, processor_register_type::l0); break;
        case 0x103F:  execute_ld_r32(mem, processor_register_type::b15, processor_register_type::l0); break;
        case 0x1040:  execute_ld_r32(mem, processor_register_type::b0,  processor_register_type::l1); break;
        case 0x1041:  execute_ld_r32(mem, processor_register_type::b1,  processor_register_type::l1); break;
        case 0x1042:  execute_ld_r32(mem, processor_register_type::b2,  processor_register_type::l1); break;
        case 0x1043:  execute_ld_r32(mem, processor_register_type::b3,  processor_register_type::l1); break;
        case 0x1044:  execute_ld_r32(mem, processor_register_type::b4,  processor_register_type::l1); break;
        case 0x1045:  execute_ld_r32(mem, processor_register_type::b5,  processor_register_type::l1); break;
        case 0x1046:  execute_ld_r32(mem, processor_register_type::b6,  processor_register_type::l1); break;
        case 0x1047:  execute_ld_r32(mem, processor_register_type::b7,  processor_register_type::l1); break;
        case 0x1048:  execute_ld_r32(mem, processor_register_type::b8,  processor_register_type::l1); break;
        case 0x1049:  execute_ld_r32(mem, processor_register_type::b9,  processor_register_type::l1); break;
        case 0x104A:  execute_ld_r32(mem, processor_register_type::b10, processor_register_type::l1); break;
        case 0x104B:  execute_ld_r32(mem, processor_register_type::b11, processor_register_type::l1); break;
        case 0x104C:  execute_ld_r32(mem, processor_register_type::b12, processor_register_type::l1); break;
        case 0x104D:  execute_ld_r32(mem, processor_register_type::b13, processor_register_type::l1); break;
        case 0x104E:  execute_ld_r32(mem, processor_register_type::b14, processor_register_type::l1); break;
        case 0x104F:  execute_ld_r32(mem, processor_register_type::b15, processor_register_type::l1); break;
        case 0x1050:  execute_ld_r32(mem, processor_register_type::b0,  processor_register_type::l2); break;
        case 0x1051:  execute_ld_r32(mem, processor_register_type::b1,  processor_register_type::l2); break;
        case 0x1052:  execute_ld_r32(mem, processor_register_type::b2,  processor_register_type::l2); break;
        case 0x1053:  execute_ld_r32(mem, processor_register_type::b3,  processor_register_type::l2); break;
        case 0x1054:  execute_ld_r32(mem, processor_register_type::b4,  processor_register_type::l2); break;
        case 0x1055:  execute_ld_r32(mem, processor_register_type::b5,  processor_register_type::l2); break;
        case 0x1056:  execute_ld_r32(mem, processor_register_type::b6,  processor_register_type::l2); break;
        case 0x1057:  execute_ld_r32(mem, processor_register_type::b7,  processor_register_type::l2); break;
        case 0x1058:  execute_ld_r32(mem, processor_register_type::b8,  processor_register_type::l2); break;
        case 0x1059:  execute_ld_r32(mem, processor_register_type::b9,  processor_register_type::l2); break;
        case 0x105A:  execute_ld_r32(mem, processor_register_type::b10, processor_register_type::l2); break;
        case 0x105B:  execute_ld_r32(mem, processor_register_type::b11, processor_register_type::l2); break;
        case 0x105C:  execute_ld_r32(mem, processor_register_type::b12, processor_register_type::l2); break;
        case 0x105D:  execute_ld_r32(mem, processor_register_type::b13, processor_register_type::l2); break;
        case 0x105E:  execute_ld_r32(mem, processor_register_type::b14, processor_register_type::l2); break;
        case 0x105F:  execute_ld_r32(mem, processor_register_type::b15, processor_register_type::l2); break;
        case 0x1060:  execute_ld_r32(mem, processor_register_type::b0,  processor_register_type::l3); break;
        case 0x1061:  execute_ld_r32(mem, processor_register_type::b1,  processor_register_type::l3); break;
        case 0x1062:  execute_ld_r32(mem, processor_register_type::b2,  processor_register_type::l3); break;
        case 0x1063:  execute_ld_r32(mem, processor_register_type::b3,  processor_register_type::l3); break;
        case 0x1064:  execute_ld_r32(mem, processor_register_type::b4,  processor_register_type::l3); break;
        case 0x1065:  execute_ld_r32(mem, processor_register_type::b5,  processor_register_type::l3); break;
        case 0x1066:  execute_ld_r32(mem, processor_register_type::b6,  processor_register_type::l3); break;
        case 0x1067:  execute_ld_r32(mem, processor_register_type::b7,  processor_register_type::l3); break;
        case 0x1068:  execute_ld_r32(mem, processor_register_type::b8,  processor_register_type::l3); break;
        case 0x1069:  execute_ld_r32(mem, processor_register_type::b9,  processor_register_type::l3); break;
        case 0x106A:  execute_ld_r32(mem, processor_register_type::b10, processor_register_type::l3); break;
        case 0x106B:  execute_ld_r32(mem, processor_register_type::b11, processor_register_type::l3); break;
        case 0x106C:  execute_ld_r32(mem, processor_register_type::b12, processor_register_type::l3); break;
        case 0x106D:  execute_ld_r32(mem, processor_register_type::b13, processor_register_type::l3); break;
        case 0x106E:  execute_ld_r32(mem, processor_register_type::b14, processor_register_type::l3); break;
        case 0x106F:  execute_ld_r32(mem, processor_register_type::b15, processor_register_type::l3); break;
        case 0x1070:  execute_lhb   (mem);  break;
        case 0x1071:  execute_lhr   (mem);  break;
        case 0x1072:  execute_lhw   (mem);  break;

        // 11XX. Data Transfer Instructions - Store Instructions
        case 0x1120:  execute_st_a32(mem, processor_register_type::b0);   break;
        case 0x1121:  execute_st_a32(mem, processor_register_type::b1);   break;
        case 0x1122:  execute_st_a32(mem, processor_register_type::b2);   break;
        case 0x1123:  execute_st_a32(mem, processor_register_type::b3);   break;
        case 0x1124:  execute_st_a32(mem, processor_register_type::b4);   break;
        case 0x1125:  execute_st_a32(mem, processor_register_type::b5);   break;
        case 0x1126:  execute_st_a32(mem, processor_register_type::b6);   break;
        case 0x1127:  execute_st_a32(mem, processor_register_type::b7);   break;
        case 0x1128:  execute_st_a32(mem, processor_register_type::b8);   break;
        case 0x1129:  execute_st_a32(mem, processor_register_type::b9);   break;
        case 0x112A:  execute_st_a32(mem, processor_register_type::b10);  break;
        case 0x112B:  execute_st_a32(mem, processor_register_type::b11);  break;
        case 0x112C:  execute_st_a32(mem, processor_register_type::b12);  break;
        case 0x112D:  execute_st_a32(mem, processor_register_type::b13);  break;
        case 0x112E:  execute_st_a32(mem, processor_register_type::b14);  break;
        case 0x112F:  execute_st_a32(mem, processor_register_type::b15);  break;
        case 0x1130:  execute_st_r32(mem, processor_register_type::b0,  processor_register_type::l0); break;
        case 0x1131:  execute_st_r32(mem, processor_register_type::b1,  processor_register_type::l0); break;
        case 0x1132:  execute_st_r32(mem, processor_register_type::b2,  processor_register_type::l0); break;
        case 0x1133:  execute_st_r32(mem, processor_register_type::b3,  processor_register_type::l0); break;
        case 0x1134:  execute_st_r32(mem, processor_register_type::b4,  processor_register_type::l0); break;
        case 0x1135:  execute_st_r32(mem, processor_register_type::b5,  processor_register_type::l0); break;
        case 0x1136:  execute_st_r32(mem, processor_register_type::b6,  processor_register_type::l0); break;
        case 0x1137:  execute_st_r32(mem, processor_register_type::b7,  processor_register_type::l0); break;
        case 0x1138:  execute_st_r32(mem, processor_register_type::b8,  processor_register_type::l0); break;
        case 0x1139:  execute_st_r32(mem, processor_register_type::b9,  processor_register_type::l0); break;
        case 0x113A:  execute_st_r32(mem, processor_register_type::b10, processor_register_type::l0); break;
        case 0x113B:  execute_st_r32(mem, processor_register_type::b11, processor_register_type::l0); break;
        case 0x113C:  execute_st_r32(mem, processor_register_type::b12, processor_register_type::l0); break;
        case 0x113D:  execute_st_r32(mem, processor_register_type::b13, processor_register_type::l0); break;
        case 0x113E:  execute_st_r32(mem, processor_register_type::b14, processor_register_type::l0); break;
        case 0x113F:  execute_st_r32(mem, processor_register_type::b15, processor_register_type::l0); break;
        case 0x1140:  execute_st_r32(mem, processor_register_type::b0,  processor_register_type::l1); break;
        case 0x1141:  execute_st_r32(mem, processor_register_type::b1,  processor_register_type::l1); break;
        case 0x1142:  execute_st_r32(mem, processor_register_type::b2,  processor_register_type::l1); break;
        case 0x1143:  execute_st_r32(mem, processor_register_type::b3,  processor_register_type::l1); break;
        case 0x1144:  execute_st_r32(mem, processor_register_type::b4,  processor_register_type::l1); break;
        case 0x1145:  execute_st_r32(mem, processor_register_type::b5,  processor_register_type::l1); break;
        case 0x1146:  execute_st_r32(mem, processor_register_type::b6,  processor_register_type::l1); break;
        case 0x1147:  execute_st_r32(mem, processor_register_type::b7,  processor_register_type::l1); break;
        case 0x1148:  execute_st_r32(mem, processor_register_type::b8,  processor_register_type::l1); break;
        case 0x1149:  execute_st_r32(mem, processor_register_type::b9,  processor_register_type::l1); break;
        case 0x114A:  execute_st_r32(mem, processor_register_type::b10, processor_register_type::l1); break;
        case 0x114B:  execute_st_r32(mem, processor_register_type::b11, processor_register_type::l1); break;
        case 0x114C:  execute_st_r32(mem, processor_register_type::b12, processor_register_type::l1); break;
        case 0x114D:  execute_st_r32(mem, processor_register_type::b13, processor_register_type::l1); break;
        case 0x114E:  execute_st_r32(mem, processor_register_type::b14, processor_register_type::l1); break;
        case 0x114F:  execute_st_r32(mem, processor_register_type::b15, processor_register_type::l1); break;
        case 0x1150:  execute_st_r32(mem, processor_register_type::b0,  processor_register_type::l2); break;
        case 0x1151:  execute_st_r32(mem, processor_register_type::b1,  processor_register_type::l2); break;
        case 0x1152:  execute_st_r32(mem, processor_register_type::b2,  processor_register_type::l2); break;
        case 0x1153:  execute_st_r32(mem, processor_register_type::b3,  processor_register_type::l2); break;
        case 0x1154:  execute_st_r32(mem, processor_register_type::b4,  processor_register_type::l2); break;
        case 0x1155:  execute_st_r32(mem, processor_register_type::b5,  processor_register_type::l2); break;
        case 0x1156:  execute_st_r32(mem, processor_register_type::b6,  processor_register_type::l2); break;
        case 0x1157:  execute_st_r32(mem, processor_register_type::b7,  processor_register_type::l2); break;
        case 0x1158:  execute_st_r32(mem, processor_register_type::b8,  processor_register_type::l2); break;
        case 0x1159:  execute_st_r32(mem, processor_register_type::b9,  processor_register_type::l2); break;
        case 0x115A:  execute_st_r32(mem, processor_register_type::b10, processor_register_type::l2); break;
        case 0x115B:  execute_st_r32(mem, processor_register_type::b11, processor_register_type::l2); break;
        case 0x115C:  execute_st_r32(mem, processor_register_type::b12, processor_register_type::l2); break;
        case 0x115D:  execute_st_r32(mem, processor_register_type::b13, processor_register_type::l2); break;
        case 0x115E:  execute_st_r32(mem, processor_register_type::b14, processor_register_type::l2); break;
        case 0x115F:  execute_st_r32(mem, processor_register_type::b15, processor_register_type::l2); break;
        case 0x1160:  execute_st_r32(mem, processor_register_type::b0,  processor_register_type::l3); break;
        case 0x1161:  execute_st_r32(mem, processor_register_type::b1,  processor_register_type::l3); break;
        case 0x1162:  execute_st_r32(mem, processor_register_type::b2,  processor_register_type::l3); break;
        case 0x1163:  execute_st_r32(mem, processor_register_type::b3,  processor_register_type::l3); break;
        case 0x1164:  execute_st_r32(mem, processor_register_type::b4,  processor_register_type::l3); break;
        case 0x1165:  execute_st_r32(mem, processor_register_type::b5,  processor_register_type::l3); break;
        case 0x1166:  execute_st_r32(mem, processor_register_type::b6,  processor_register_type::l3); break;
        case 0x1167:  execute_st_r32(mem, processor_register_type::b7,  processor_register_type::l3); break;
        case 0x1168:  execute_st_r32(mem, processor_register_type::b8,  processor_register_type::l3); break;
        case 0x1169:  execute_st_r32(mem, processor_register_type::b9,  processor_register_type::l3); break;
        case 0x116A:  execute_st_r32(mem, processor_register_type::b10, processor_register_type::l3); break;
        case 0x116B:  execute_st_r32(mem, processor_register_type::b11, processor_register_type::l3); break;
        case 0x116C:  execute_st_r32(mem, processor_register_type::b12, processor_register_type::l3); break;
        case 0x116D:  execute_st_r32(mem, processor_register_type::b13, processor_register_type::l3); break;
        case 0x116E:  execute_st_r32(mem, processor_register_type::b14, processor_register_type::l3); break;
        case 0x116F:  execute_st_r32(mem, processor_register_type::b15, processor_register_type::l3); break;
        case 0x1170:  execute_shb   (mem);  break;
        case 0x1171:  execute_shr   (mem);  break;
        case 0x1172:  execute_shw   (mem);  break;
        case 0x1173:  execute_ssp   (mem);  break;
        case 0x1174:  execute_spc   (mem);  break;

        // 12XX. Data Transfer Instructions - Direct Byte Register Move Instructions

        case 0x1200:  execute_mv(processor_register_type::b0, processor_register_type::b0);     break;
        case 0x1201:  execute_mv(processor_register_type::b0, processor_register_type::b1);     break;
        case 0x1202:  execute_mv(processor_register_type::b0, processor_register_type::b2);     break;
        case 0x1203:  execute_mv(processor_register_type::b0, processor_register_type::b3);     break;
        case 0x1204:  execute_mv(processor_register_type::b0, processor_register_type::b4);     break;
        case 0x1205:  execute_mv(processor_register_type::b0, processor_register_type::b5);     break;
        case 0x1206:  execute_mv(processor_register_type::b0, processor_register_type::b6);     break;
        case 0x1207:  execute_mv(processor_register_type::b0, processor_register_type::b7);     break;
        case 0x1208:  execute_mv(processor_register_type::b0, processor_register_type::b8);     break;
        case 0x1209:  execute_mv(processor_register_type::b0, processor_register_type::b9);     break;
        case 0x120A:  execute_mv(processor_register_type::b0, processor_register_type::b10);    break;
        case 0x120B:  execute_mv(processor_register_type::b0, processor_register_type::b11);    break;
        case 0x120C:  execute_mv(processor_register_type::b0, processor_register_type::b12);    break;
        case 0x120D:  execute_mv(processor_register_type::b0, processor_register_type::b13);    break;
        case 0x120E:  execute_mv(processor_register_type::b0, processor_register_type::b14);    break;
        case 0x120F:  execute_mv(processor_register_type::b0, processor_register_type::b15);    break;
        case 0x1210:  execute_mv(processor_register_type::b1, processor_register_type::b0);     break;
        case 0x1211:  execute_mv(processor_register_type::b1, processor_register_type::b1);     break;
        case 0x1212:  execute_mv(processor_register_type::b1, processor_register_type::b2);     break;
        case 0x1213:  execute_mv(processor_register_type::b1, processor_register_type::b3);     break;
        case 0x1214:  execute_mv(processor_register_type::b1, processor_register_type::b4);     break;
        case 0x1215:  execute_mv(processor_register_type::b1, processor_register_type::b5);     break;
        case 0x1216:  execute_mv(processor_register_type::b1, processor_register_type::b6);     break;
        case 0x1217:  execute_mv(processor_register_type::b1, processor_register_type::b7);     break;
        case 0x1218:  execute_mv(processor_register_type::b1, processor_register_type::b8);     break;
        case 0x1219:  execute_mv(processor_register_type::b1, processor_register_type::b9);     break;
        case 0x121A:  execute_mv(processor_register_type::b1, processor_register_type::b10);    break;
        case 0x121B:  execute_mv(processor_register_type::b1, processor_register_type::b11);    break;
        case 0x121C:  execute_mv(processor_register_type::b1, processor_register_type::b12);    break;
        case 0x121D:  execute_mv(processor_register_type::b1, processor_register_type::b13);    break;
        case 0x121E:  execute_mv(processor_register_type::b1, processor_register_type::b14);    break;
        case 0x121F:  execute_mv(processor_register_type::b1, processor_register_type::b15);    break;
        case 0x1220:  execute_mv(processor_register_type::b2, processor_register_type::b0);     break;
        case 0x1221:  execute_mv(processor_register_type::b2, processor_register_type::b1);     break;
        case 0x1222:  execute_mv(processor_register_type::b2, processor_register_type::b2);     break;
        case 0x1223:  execute_mv(processor_register_type::b2, processor_register_type::b3);     break;
        case 0x1224:  execute_mv(processor_register_type::b2, processor_register_type::b4);     break;
        case 0x1225:  execute_mv(processor_register_type::b2, processor_register_type::b5);     break;
        case 0x1226:  execute_mv(processor_register_type::b2, processor_register_type::b6);     break;
        case 0x1227:  execute_mv(processor_register_type::b2, processor_register_type::b7);     break;
        case 0x1228:  execute_mv(processor_register_type::b2, processor_register_type::b8);     break;
        case 0x1229:  execute_mv(processor_register_type::b2, processor_register_type::b9);     break;
        case 0x122A:  execute_mv(processor_register_type::b2, processor_register_type::b10);    break;
        case 0x122B:  execute_mv(processor_register_type::b2, processor_register_type::b11);    break;
        case 0x122C:  execute_mv(processor_register_type::b2, processor_register_type::b12);    break;
        case 0x122D:  execute_mv(processor_register_type::b2, processor_register_type::b13);    break;
        case 0x122E:  execute_mv(processor_register_type::b2, processor_register_type::b14);    break;
        case 0x122F:  execute_mv(processor_register_type::b2, processor_register_type::b15);    break;
        case 0x1230:  execute_mv(processor_register_type::b3, processor_register_type::b0);     break;
        case 0x1231:  execute_mv(processor_register_type::b3, processor_register_type::b1);     break;
        case 0x1232:  execute_mv(processor_register_type::b3, processor_register_type::b2);     break;
        case 0x1233:  execute_mv(processor_register_type::b3, processor_register_type::b3);     break;
        case 0x1234:  execute_mv(processor_register_type::b3, processor_register_type::b4);     break;
        case 0x1235:  execute_mv(processor_register_type::b3, processor_register_type::b5);     break;
        case 0x1236:  execute_mv(processor_register_type::b3, processor_register_type::b6);     break;
        case 0x1237:  execute_mv(processor_register_type::b3, processor_register_type::b7);     break;
        case 0x1238:  execute_mv(processor_register_type::b3, processor_register_type::b8);     break;
        case 0x1239:  execute_mv(processor_register_type::b3, processor_register_type::b9);     break;
        case 0x123A:  execute_mv(processor_register_type::b3, processor_register_type::b10);    break;
        case 0x123B:  execute_mv(processor_register_type::b3, processor_register_type::b11);    break;
        case 0x123C:  execute_mv(processor_register_type::b3, processor_register_type::b12);    break;
        case 0x123D:  execute_mv(processor_register_type::b3, processor_register_type::b13);    break;
        case 0x123E:  execute_mv(processor_register_type::b3, processor_register_type::b14);    break;
        case 0x123F:  execute_mv(processor_register_type::b3, processor_register_type::b15);    break;
        case 0x1240:  execute_mv(processor_register_type::b4, processor_register_type::b0);     break;
        case 0x1241:  execute_mv(processor_register_type::b4, processor_register_type::b1);     break;
        case 0x1242:  execute_mv(processor_register_type::b4, processor_register_type::b2);     break;
        case 0x1243:  execute_mv(processor_register_type::b4, processor_register_type::b3);     break;
        case 0x1244:  execute_mv(processor_register_type::b4, processor_register_type::b4);     break;
        case 0x1245:  execute_mv(processor_register_type::b4, processor_register_type::b5);     break;
        case 0x1246:  execute_mv(processor_register_type::b4, processor_register_type::b6);     break;
        case 0x1247:  execute_mv(processor_register_type::b4, processor_register_type::b7);     break;
        case 0x1248:  execute_mv(processor_register_type::b4, processor_register_type::b8);     break;
        case 0x1249:  execute_mv(processor_register_type::b4, processor_register_type::b9);     break;
        case 0x124A:  execute_mv(processor_register_type::b4, processor_register_type::b10);    break;
        case 0x124B:  execute_mv(processor_register_type::b4, processor_register_type::b11);    break;
        case 0x124C:  execute_mv(processor_register_type::b4, processor_register_type::b12);    break;
        case 0x124D:  execute_mv(processor_register_type::b4, processor_register_type::b13);    break;
        case 0x124E:  execute_mv(processor_register_type::b4, processor_register_type::b14);    break;
        case 0x124F:  execute_mv(processor_register_type::b4, processor_register_type::b15);    break;
        case 0x1250:  execute_mv(processor_register_type::b5, processor_register_type::b0);     break;
        case 0x1251:  execute_mv(processor_register_type::b5, processor_register_type::b1);     break;
        case 0x1252:  execute_mv(processor_register_type::b5, processor_register_type::b2);     break;
        case 0x1253:  execute_mv(processor_register_type::b5, processor_register_type::b3);     break;
        case 0x1254:  execute_mv(processor_register_type::b5, processor_register_type::b4);     break;
        case 0x1255:  execute_mv(processor_register_type::b5, processor_register_type::b5);     break;
        case 0x1256:  execute_mv(processor_register_type::b5, processor_register_type::b6);     break;
        case 0x1257:  execute_mv(processor_register_type::b5, processor_register_type::b7);     break;
        case 0x1258:  execute_mv(processor_register_type::b5, processor_register_type::b8);     break;
        case 0x1259:  execute_mv(processor_register_type::b5, processor_register_type::b9);     break;
        case 0x125A:  execute_mv(processor_register_type::b5, processor_register_type::b10);    break;
        case 0x125B:  execute_mv(processor_register_type::b5, processor_register_type::b11);    break;
        case 0x125C:  execute_mv(processor_register_type::b5, processor_register_type::b12);    break;
        case 0x125D:  execute_mv(processor_register_type::b5, processor_register_type::b13);    break;
        case 0x125E:  execute_mv(processor_register_type::b5, processor_register_type::b14);    break;
        case 0x125F:  execute_mv(processor_register_type::b5, processor_register_type::b15);    break;
        case 0x1260:  execute_mv(processor_register_type::b6, processor_register_type::b0);     break;
        case 0x1261:  execute_mv(processor_register_type::b6, processor_register_type::b1);     break;
        case 0x1262:  execute_mv(processor_register_type::b6, processor_register_type::b2);     break;
        case 0x1263:  execute_mv(processor_register_type::b6, processor_register_type::b3);     break;
        case 0x1264:  execute_mv(processor_register_type::b6, processor_register_type::b4);     break;
        case 0x1265:  execute_mv(processor_register_type::b6, processor_register_type::b5);     break;
        case 0x1266:  execute_mv(processor_register_type::b6, processor_register_type::b6);     break;
        case 0x1267:  execute_mv(processor_register_type::b6, processor_register_type::b7);     break;
        case 0x1268:  execute_mv(processor_register_type::b6, processor_register_type::b8);     break;
        case 0x1269:  execute_mv(processor_register_type::b6, processor_register_type::b9);     break;
        case 0x126A:  execute_mv(processor_register_type::b6, processor_register_type::b10);    break;
        case 0x126B:  execute_mv(processor_register_type::b6, processor_register_type::b11);    break;
        case 0x126C:  execute_mv(processor_register_type::b6, processor_register_type::b12);    break;
        case 0x126D:  execute_mv(processor_register_type::b6, processor_register_type::b13);    break;
        case 0x126E:  execute_mv(processor_register_type::b6, processor_register_type::b14);    break;
        case 0x126F:  execute_mv(processor_register_type::b6, processor_register_type::b15);    break;
        case 0x1270:  execute_mv(processor_register_type::b7, processor_register_type::b0);     break;
        case 0x1271:  execute_mv(processor_register_type::b7, processor_register_type::b1);     break;
        case 0x1272:  execute_mv(processor_register_type::b7, processor_register_type::b2);     break;
        case 0x1273:  execute_mv(processor_register_type::b7, processor_register_type::b3);     break;
        case 0x1274:  execute_mv(processor_register_type::b7, processor_register_type::b4);     break;
        case 0x1275:  execute_mv(processor_register_type::b7, processor_register_type::b5);     break;
        case 0x1276:  execute_mv(processor_register_type::b7, processor_register_type::b6);     break;
        case 0x1277:  execute_mv(processor_register_type::b7, processor_register_type::b7);     break;
        case 0x1278:  execute_mv(processor_register_type::b7, processor_register_type::b8);     break;
        case 0x1279:  execute_mv(processor_register_type::b7, processor_register_type::b9);     break;
        case 0x127A:  execute_mv(processor_register_type::b7, processor_register_type::b10);    break;
        case 0x127B:  execute_mv(processor_register_type::b7, processor_register_type::b11);    break;
        case 0x127C:  execute_mv(processor_register_type::b7, processor_register_type::b12);    break;
        case 0x127D:  execute_mv(processor_register_type::b7, processor_register_type::b13);    break;
        case 0x127E:  execute_mv(processor_register_type::b7, processor_register_type::b14);    break;
        case 0x127F:  execute_mv(processor_register_type::b7, processor_register_type::b15);    break;
        case 0x1280:  execute_mv(processor_register_type::b8, processor_register_type::b0);     break;
        case 0x1281:  execute_mv(processor_register_type::b8, processor_register_type::b1);     break;
        case 0x1282:  execute_mv(processor_register_type::b8, processor_register_type::b2);     break;
        case 0x1283:  execute_mv(processor_register_type::b8, processor_register_type::b3);     break;
        case 0x1284:  execute_mv(processor_register_type::b8, processor_register_type::b4);     break;
        case 0x1285:  execute_mv(processor_register_type::b8, processor_register_type::b5);     break;
        case 0x1286:  execute_mv(processor_register_type::b8, processor_register_type::b6);     break;
        case 0x1287:  execute_mv(processor_register_type::b8, processor_register_type::b7);     break;
        case 0x1288:  execute_mv(processor_register_type::b8, processor_register_type::b8);     break;
        case 0x1289:  execute_mv(processor_register_type::b8, processor_register_type::b9);     break;
        case 0x128A:  execute_mv(processor_register_type::b8, processor_register_type::b10);    break;
        case 0x128B:  execute_mv(processor_register_type::b8, processor_register_type::b11);    break;
        case 0x128C:  execute_mv(processor_register_type::b8, processor_register_type::b12);    break;
        case 0x128D:  execute_mv(processor_register_type::b8, processor_register_type::b13);    break;
        case 0x128E:  execute_mv(processor_register_type::b8, processor_register_type::b14);    break;
        case 0x128F:  execute_mv(processor_register_type::b8, processor_register_type::b15);    break;
        case 0x1290:  execute_mv(processor_register_type::b9, processor_register_type::b0);     break;
        case 0x1291:  execute_mv(processor_register_type::b9, processor_register_type::b1);     break;
        case 0x1292:  execute_mv(processor_register_type::b9, processor_register_type::b2);     break;
        case 0x1293:  execute_mv(processor_register_type::b9, processor_register_type::b3);     break;
        case 0x1294:  execute_mv(processor_register_type::b9, processor_register_type::b4);     break;
        case 0x1295:  execute_mv(processor_register_type::b9, processor_register_type::b5);     break;
        case 0x1296:  execute_mv(processor_register_type::b9, processor_register_type::b6);     break;
        case 0x1297:  execute_mv(processor_register_type::b9, processor_register_type::b7);     break;
        case 0x1298:  execute_mv(processor_register_type::b9, processor_register_type::b8);     break;
        case 0x1299:  execute_mv(processor_register_type::b9, processor_register_type::b9);     break;
        case 0x129A:  execute_mv(processor_register_type::b9, processor_register_type::b10);    break;
        case 0x129B:  execute_mv(processor_register_type::b9, processor_register_type::b11);    break;
        case 0x129C:  execute_mv(processor_register_type::b9, processor_register_type::b12);    break;
        case 0x129D:  execute_mv(processor_register_type::b9, processor_register_type::b13);    break;
        case 0x129E:  execute_mv(processor_register_type::b9, processor_register_type::b14);    break;
        case 0x129F:  execute_mv(processor_register_type::b9, processor_register_type::b15);    break;
        case 0x12A0:  execute_mv(processor_register_type::b10, processor_register_type::b0);    break;
        case 0x12A1:  execute_mv(processor_register_type::b10, processor_register_type::b1);    break;
        case 0x12A2:  execute_mv(processor_register_type::b10, processor_register_type::b2);    break;
        case 0x12A3:  execute_mv(processor_register_type::b10, processor_register_type::b3);    break;
        case 0x12A4:  execute_mv(processor_register_type::b10, processor_register_type::b4);    break;
        case 0x12A5:  execute_mv(processor_register_type::b10, processor_register_type::b5);    break;
        case 0x12A6:  execute_mv(processor_register_type::b10, processor_register_type::b6);    break;
        case 0x12A7:  execute_mv(processor_register_type::b10, processor_register_type::b7);    break;
        case 0x12A8:  execute_mv(processor_register_type::b10, processor_register_type::b8);    break;
        case 0x12A9:  execute_mv(processor_register_type::b10, processor_register_type::b9);    break;
        case 0x12AA:  execute_mv(processor_register_type::b10, processor_register_type::b10);   break;
        case 0x12AB:  execute_mv(processor_register_type::b10, processor_register_type::b11);   break;
        case 0x12AC:  execute_mv(processor_register_type::b10, processor_register_type::b12);   break;
        case 0x12AD:  execute_mv(processor_register_type::b10, processor_register_type::b13);   break;
        case 0x12AE:  execute_mv(processor_register_type::b10, processor_register_type::b14);   break;
        case 0x12AF:  execute_mv(processor_register_type::b10, processor_register_type::b15);   break;
        case 0x12B0:  execute_mv(processor_register_type::b11, processor_register_type::b0);    break;
        case 0x12B1:  execute_mv(processor_register_type::b11, processor_register_type::b1);    break;
        case 0x12B2:  execute_mv(processor_register_type::b11, processor_register_type::b2);    break;
        case 0x12B3:  execute_mv(processor_register_type::b11, processor_register_type::b3);    break;
        case 0x12B4:  execute_mv(processor_register_type::b11, processor_register_type::b4);    break;
        case 0x12B5:  execute_mv(processor_register_type::b11, processor_register_type::b5);    break;
        case 0x12B6:  execute_mv(processor_register_type::b11, processor_register_type::b6);    break;
        case 0x12B7:  execute_mv(processor_register_type::b11, processor_register_type::b7);    break;
        case 0x12B8:  execute_mv(processor_register_type::b11, processor_register_type::b8);    break;
        case 0x12B9:  execute_mv(processor_register_type::b11, processor_register_type::b9);    break;
        case 0x12BA:  execute_mv(processor_register_type::b11, processor_register_type::b10);   break;
        case 0x12BB:  execute_mv(processor_register_type::b11, processor_register_type::b11);   break;
        case 0x12BC:  execute_mv(processor_register_type::b11, processor_register_type::b12);   break;
        case 0x12BD:  execute_mv(processor_register_type::b11, processor_register_type::b13);   break;
        case 0x12BE:  execute_mv(processor_register_type::b11, processor_register_type::b14);   break;
        case 0x12BF:  execute_mv(processor_register_type::b11, processor_register_type::b15);   break;
        case 0x12C0:  execute_mv(processor_register_type::b12, processor_register_type::b0);    break;
        case 0x12C1:  execute_mv(processor_register_type::b12, processor_register_type::b1);    break;
        case 0x12C2:  execute_mv(processor_register_type::b12, processor_register_type::b2);    break;
        case 0x12C3:  execute_mv(processor_register_type::b12, processor_register_type::b3);    break;
        case 0x12C4:  execute_mv(processor_register_type::b12, processor_register_type::b4);    break;
        case 0x12C5:  execute_mv(processor_register_type::b12, processor_register_type::b5);    break;
        case 0x12C6:  execute_mv(processor_register_type::b12, processor_register_type::b6);    break;
        case 0x12C7:  execute_mv(processor_register_type::b12, processor_register_type::b7);    break;
        case 0x12C8:  execute_mv(processor_register_type::b12, processor_register_type::b8);    break;
        case 0x12C9:  execute_mv(processor_register_type::b12, processor_register_type::b9);    break;
        case 0x12CA:  execute_mv(processor_register_type::b12, processor_register_type::b10);   break;
        case 0x12CB:  execute_mv(processor_register_type::b12, processor_register_type::b11);   break;
        case 0x12CC:  execute_mv(processor_register_type::b12, processor_register_type::b12);   break;
        case 0x12CD:  execute_mv(processor_register_type::b12, processor_register_type::b13);   break;
        case 0x12CE:  execute_mv(processor_register_type::b12, processor_register_type::b14);   break;
        case 0x12CF:  execute_mv(processor_register_type::b12, processor_register_type::b15);   break;
        case 0x12D0:  execute_mv(processor_register_type::b13, processor_register_type::b0);    break;
        case 0x12D1:  execute_mv(processor_register_type::b13, processor_register_type::b1);    break;
        case 0x12D2:  execute_mv(processor_register_type::b13, processor_register_type::b2);    break;
        case 0x12D3:  execute_mv(processor_register_type::b13, processor_register_type::b3);    break;
        case 0x12D4:  execute_mv(processor_register_type::b13, processor_register_type::b4);    break;
        case 0x12D5:  execute_mv(processor_register_type::b13, processor_register_type::b5);    break;
        case 0x12D6:  execute_mv(processor_register_type::b13, processor_register_type::b6);    break;
        case 0x12D7:  execute_mv(processor_register_type::b13, processor_register_type::b7);    break;
        case 0x12D8:  execute_mv(processor_register_type::b13, processor_register_type::b8);    break;
        case 0x12D9:  execute_mv(processor_register_type::b13, processor_register_type::b9);    break;
        case 0x12DA:  execute_mv(processor_register_type::b13, processor_register_type::b10);   break;
        case 0x12DB:  execute_mv(processor_register_type::b13, processor_register_type::b11);   break;
        case 0x12DC:  execute_mv(processor_register_type::b13, processor_register_type::b12);   break;
        case 0x12DD:  execute_mv(processor_register_type::b13, processor_register_type::b13);   break;
        case 0x12DE:  execute_mv(processor_register_type::b13, processor_register_type::b14);   break;
        case 0x12DF:  execute_mv(processor_register_type::b13, processor_register_type::b15);   break;
        case 0x12E0:  execute_mv(processor_register_type::b14, processor_register_type::b0);    break;
        case 0x12E1:  execute_mv(processor_register_type::b14, processor_register_type::b1);    break;
        case 0x12E2:  execute_mv(processor_register_type::b14, processor_register_type::b2);    break;
        case 0x12E3:  execute_mv(processor_register_type::b14, processor_register_type::b3);    break;
        case 0x12E4:  execute_mv(processor_register_type::b14, processor_register_type::b4);    break;
        case 0x12E5:  execute_mv(processor_register_type::b14, processor_register_type::b5);    break;
        case 0x12E6:  execute_mv(processor_register_type::b14, processor_register_type::b6);    break;
        case 0x12E7:  execute_mv(processor_register_type::b14, processor_register_type::b7);    break;
        case 0x12E8:  execute_mv(processor_register_type::b14, processor_register_type::b8);    break;
        case 0x12E9:  execute_mv(processor_register_type::b14, processor_register_type::b9);    break;
        case 0x12EA:  execute_mv(processor_register_type::b14, processor_register_type::b10);   break;
        case 0x12EB:  execute_mv(processor_register_type::b14, processor_register_type::b11);   break;
        case 0x12EC:  execute_mv(processor_register_type::b14, processor_register_type::b12);   break;
        case 0x12ED:  execute_mv(processor_register_type::b14, processor_register_type::b13);   break;
        case 0x12EE:  execute_mv(processor_register_type::b14, processor_register_type::b14);   break;
        case 0x12EF:  execute_mv(processor_register_type::b14, processor_register_type::b15);   break;
        case 0x12F0:  execute_mv(processor_register_type::b15, processor_register_type::b0);    break;
        case 0x12F1:  execute_mv(processor_register_type::b15, processor_register_type::b1);    break;
        case 0x12F2:  execute_mv(processor_register_type::b15, processor_register_type::b2);    break;
        case 0x12F3:  execute_mv(processor_register_type::b15, processor_register_type::b3);    break;
        case 0x12F4:  execute_mv(processor_register_type::b15, processor_register_type::b4);    break;
        case 0x12F5:  execute_mv(processor_register_type::b15, processor_register_type::b5);    break;
        case 0x12F6:  execute_mv(processor_register_type::b15, processor_register_type::b6);    break;
        case 0x12F7:  execute_mv(processor_register_type::b15, processor_register_type::b7);    break;
        case 0x12F8:  execute_mv(processor_register_type::b15, processor_register_type::b8);    break;
        case 0x12F9:  execute_mv(processor_register_type::b15, processor_register_type::b9);    break;
        case 0x12FA:  execute_mv(processor_register_type::b15, processor_register_type::b10);   break;
        case 0x12FB:  execute_mv(processor_register_type::b15, processor_register_type::b11);   break;
        case 0x12FC:  execute_mv(processor_register_type::b15, processor_register_type::b12);   break;
        case 0x12FD:  execute_mv(processor_register_type::b15, processor_register_type::b13);   break;
        case 0x12FE:  execute_mv(processor_register_type::b15, processor_register_type::b14);   break;
        case 0x12FF:  execute_mv(processor_register_type::b15, processor_register_type::b15);   break;

        // 13XX. Data Transfer Instructions - Indirect Word Register Move Instructions

        case 0x1300:  execute_mv(processor_register_type::w0, processor_register_type::w0);     break;
        case 0x1301:  execute_mv(processor_register_type::w0, processor_register_type::w1);     break;
        case 0x1302:  execute_mv(processor_register_type::w0, processor_register_type::w2);     break;
        case 0x1303:  execute_mv(processor_register_type::w0, processor_register_type::w3);     break;
        case 0x1304:  execute_mv(processor_register_type::w0, processor_register_type::w4);     break;
        case 0x1305:  execute_mv(processor_register_type::w0, processor_register_type::w5);     break;
        case 0x1306:  execute_mv(processor_register_type::w0, processor_register_type::w6);     break;
        case 0x1307:  execute_mv(processor_register_type::w0, processor_register_type::w7);     break;
        case 0x1310:  execute_mv(processor_register_type::w1, processor_register_type::w0);     break;
        case 0x1311:  execute_mv(processor_register_type::w1, processor_register_type::w1);     break;
        case 0x1312:  execute_mv(processor_register_type::w1, processor_register_type::w2);     break;
        case 0x1313:  execute_mv(processor_register_type::w1, processor_register_type::w3);     break;
        case 0x1314:  execute_mv(processor_register_type::w1, processor_register_type::w4);     break;
        case 0x1315:  execute_mv(processor_register_type::w1, processor_register_type::w5);     break;
        case 0x1316:  execute_mv(processor_register_type::w1, processor_register_type::w6);     break;
        case 0x1317:  execute_mv(processor_register_type::w1, processor_register_type::w7);     break;
        case 0x1320:  execute_mv(processor_register_type::w2, processor_register_type::w0);     break;
        case 0x1321:  execute_mv(processor_register_type::w2, processor_register_type::w1);     break;
        case 0x1322:  execute_mv(processor_register_type::w2, processor_register_type::w2);     break;
        case 0x1323:  execute_mv(processor_register_type::w2, processor_register_type::w3);     break;
        case 0x1324:  execute_mv(processor_register_type::w2, processor_register_type::w4);     break;
        case 0x1325:  execute_mv(processor_register_type::w2, processor_register_type::w5);     break;
        case 0x1326:  execute_mv(processor_register_type::w2, processor_register_type::w6);     break;
        case 0x1327:  execute_mv(processor_register_type::w2, processor_register_type::w7);     break;
        case 0x1330:  execute_mv(processor_register_type::w3, processor_register_type::w0);     break;
        case 0x1331:  execute_mv(processor_register_type::w3, processor_register_type::w1);     break;
        case 0x1332:  execute_mv(processor_register_type::w3, processor_register_type::w2);     break;
        case 0x1333:  execute_mv(processor_register_type::w3, processor_register_type::w3);     break;
        case 0x1334:  execute_mv(processor_register_type::w3, processor_register_type::w4);     break;
        case 0x1335:  execute_mv(processor_register_type::w3, processor_register_type::w5);     break;
        case 0x1336:  execute_mv(processor_register_type::w3, processor_register_type::w6);     break;
        case 0x1337:  execute_mv(processor_register_type::w3, processor_register_type::w7);     break;
        case 0x1340:  execute_mv(processor_register_type::w4, processor_register_type::w0);     break;
        case 0x1341:  execute_mv(processor_register_type::w4, processor_register_type::w1);     break;
        case 0x1342:  execute_mv(processor_register_type::w4, processor_register_type::w2);     break;
        case 0x1343:  execute_mv(processor_register_type::w4, processor_register_type::w3);     break;
        case 0x1344:  execute_mv(processor_register_type::w4, processor_register_type::w4);     break;
        case 0x1345:  execute_mv(processor_register_type::w4, processor_register_type::w5);     break;
        case 0x1346:  execute_mv(processor_register_type::w4, processor_register_type::w6);     break;
        case 0x1347:  execute_mv(processor_register_type::w4, processor_register_type::w7);     break;
        case 0x1350:  execute_mv(processor_register_type::w5, processor_register_type::w0);     break;
        case 0x1351:  execute_mv(processor_register_type::w5, processor_register_type::w1);     break;
        case 0x1352:  execute_mv(processor_register_type::w5, processor_register_type::w2);     break;
        case 0x1353:  execute_mv(processor_register_type::w5, processor_register_type::w3);     break;
        case 0x1354:  execute_mv(processor_register_type::w5, processor_register_type::w4);     break;
        case 0x1355:  execute_mv(processor_register_type::w5, processor_register_type::w5);     break;
        case 0x1356:  execute_mv(processor_register_type::w5, processor_register_type::w6);     break;
        case 0x1357:  execute_mv(processor_register_type::w5, processor_register_type::w7);     break;
        case 0x1360:  execute_mv(processor_register_type::w6, processor_register_type::w0);     break;
        case 0x1361:  execute_mv(processor_register_type::w6, processor_register_type::w1);     break;
        case 0x1362:  execute_mv(processor_register_type::w6, processor_register_type::w2);     break;
        case 0x1363:  execute_mv(processor_register_type::w6, processor_register_type::w3);     break;
        case 0x1364:  execute_mv(processor_register_type::w6, processor_register_type::w4);     break;
        case 0x1365:  execute_mv(processor_register_type::w6, processor_register_type::w5);     break;
        case 0x1366:  execute_mv(processor_register_type::w6, processor_register_type::w6);     break;
        case 0x1367:  execute_mv(processor_register_type::w6, processor_register_type::w7);     break;
        case 0x1370:  execute_mv(processor_register_type::w7, processor_register_type::w0);     break;
        case 0x1371:  execute_mv(processor_register_type::w7, processor_register_type::w1);     break;
        case 0x1372:  execute_mv(processor_register_type::w7, processor_register_type::w2);     break;
        case 0x1373:  execute_mv(processor_register_type::w7, processor_register_type::w3);     break;
        case 0x1374:  execute_mv(processor_register_type::w7, processor_register_type::w4);     break;
        case 0x1375:  execute_mv(processor_register_type::w7, processor_register_type::w5);     break;
        case 0x1376:  execute_mv(processor_register_type::w7, processor_register_type::w6);     break;
        case 0x1377:  execute_mv(processor_register_type::w7, processor_register_type::w7);     break;

        // 14XX. Data Transfer Instructions - Indirect Long Register Move Instructions

        case 0x1400:  execute_mv(processor_register_type::l0, processor_register_type::l0);     break;
        case 0x1401:  execute_mv(processor_register_type::l0, processor_register_type::l1);     break;
        case 0x1402:  execute_mv(processor_register_type::l0, processor_register_type::l2);     break;
        case 0x1403:  execute_mv(processor_register_type::l0, processor_register_type::l3);     break;
        case 0x1410:  execute_mv(processor_register_type::l1, processor_register_type::l0);     break;
        case 0x1411:  execute_mv(processor_register_type::l1, processor_register_type::l1);     break;
        case 0x1412:  execute_mv(processor_register_type::l1, processor_register_type::l2);     break;
        case 0x1413:  execute_mv(processor_register_type::l1, processor_register_type::l3);     break;
        case 0x1420:  execute_mv(processor_register_type::l2, processor_register_type::l0);     break;
        case 0x1421:  execute_mv(processor_register_type::l2, processor_register_type::l1);     break;
        case 0x1422:  execute_mv(processor_register_type::l2, processor_register_type::l2);     break;
        case 0x1423:  execute_mv(processor_register_type::l2, processor_register_type::l3);     break;
        case 0x1430:  execute_mv(processor_register_type::l3, processor_register_type::l0);     break;
        case 0x1431:  execute_mv(processor_register_type::l3, processor_register_type::l1);     break;
        case 0x1432:  execute_mv(processor_register_type::l3, processor_register_type::l2);     break;
        case 0x1433:  execute_mv(processor_register_type::l3, processor_register_type::l3);     break;

        // 15XX. Data Transfer Instructions - Special Purpose Move Instructions

        case 0x1500:  execute_msp(processor_register_type::w0);   break;
        case 0x1501:  execute_msp(processor_register_type::w1);   break;
        case 0x1502:  execute_msp(processor_register_type::w2);   break;
        case 0x1503:  execute_msp(processor_register_type::w3);   break;
        case 0x1504:  execute_msp(processor_register_type::w4);   break;
        case 0x1505:  execute_msp(processor_register_type::w5);   break;
        case 0x1506:  execute_msp(processor_register_type::w6);   break;
        case 0x1507:  execute_msp(processor_register_type::w7);   break;
        case 0x1508:  execute_mpc(processor_register_type::l0);   break;
        case 0x1509:  execute_mpc(processor_register_type::l1);   break;
        case 0x150A:  execute_mpc(processor_register_type::l2);   break;
        case 0x150B:  execute_mpc(processor_register_type::l3);   break;

        // 16XX. Data Transfer Instructions - Stack Instructions

        case 0x1618:  execute_push(mem, processor_register_type::l0); break;
        case 0x1619:  execute_push(mem, processor_register_type::l1); break;
        case 0x161A:  execute_push(mem, processor_register_type::l2); break;
        case 0x161B:  execute_push(mem, processor_register_type::l3); break;
        case 0x1638:  execute_pop (mem, processor_register_type::l0); break;
        case 0x1639:  execute_pop (mem, processor_register_type::l1); break;
        case 0x163A:  execute_pop (mem, processor_register_type::l2); break;
        case 0x163B:  execute_pop (mem, processor_register_type::l3); break;

        // 20XX. Control Transfer Instructions - Jumps

        case 0x2000:  execute_jmp_a32(mem, processor_condition_type::none); break;
        case 0x2001:  execute_jmp_a32(mem, processor_condition_type::z);    break;
        case 0x2002:  execute_jmp_a32(mem, processor_condition_type::nz);   break;
        case 0x2003:  execute_jmp_a32(mem, processor_condition_type::c);    break;
        case 0x2004:  execute_jmp_a32(mem, processor_condition_type::nc);   break;

        case 0x2010:  execute_jmp_r32(processor_condition_type::none, processor_register_type::l0); break;
        case 0x2011:  execute_jmp_r32(processor_condition_type::z,    processor_register_type::l0); break;
        case 0x2012:  execute_jmp_r32(processor_condition_type::nz,   processor_register_type::l0); break;
        case 0x2013:  execute_jmp_r32(processor_condition_type::c,    processor_register_type::l0); break;
        case 0x2014:  execute_jmp_r32(processor_condition_type::nc,   processor_register_type::l0); break;

        case 0x2020:  execute_jmp_r32(processor_condition_type::none, processor_register_type::l1); break;
        case 0x2021:  execute_jmp_r32(processor_condition_type::z,    processor_register_type::l1); break;
        case 0x2022:  execute_jmp_r32(processor_condition_type::nz,   processor_register_type::l1); break;
        case 0x2023:  execute_jmp_r32(processor_condition_type::c,    processor_register_type::l1); break;
        case 0x2024:  execute_jmp_r32(processor_condition_type::nc,   processor_register_type::l1); break;

        case 0x2030:  execute_jmp_r32(processor_condition_type::none, processor_register_type::l2); break;
        case 0x2031:  execute_jmp_r32(processor_condition_type::z,    processor_register_type::l2); break;
        case 0x2032:  execute_jmp_r32(processor_condition_type::nz,   processor_register_type::l2); break;
        case 0x2033:  execute_jmp_r32(processor_condition_type::c,    processor_register_type::l2); break;
        case 0x2034:  execute_jmp_r32(processor_condition_type::nc,   processor_register_type::l2); break;

        case 0x2040:  execute_jmp_r32(processor_condition_type::none, processor_register_type::l3); break;
        case 0x2041:  execute_jmp_r32(processor_condition_type::z,    processor_register_type::l3); break;
        case 0x2042:  execute_jmp_r32(processor_condition_type::nz,   processor_register_type::l3); break;
        case 0x2043:  execute_jmp_r32(processor_condition_type::c,    processor_register_type::l3); break;
        case 0x2044:  execute_jmp_r32(processor_condition_type::nc,   processor_register_type::l3); break;

        // 22XX. Control Transfer Instructions - Calls

        case 0x2200:  execute_call_a32(mem, processor_condition_type::none);  break;
        case 0x2201:  execute_call_a32(mem, processor_condition_type::z);     break;
        case 0x2202:  execute_call_a32(mem, processor_condition_type::nz);    break;
        case 0x2203:  execute_call_a32(mem, processor_condition_type::c);     break;
        case 0x2204:  execute_call_a32(mem, processor_condition_type::nc);    break;
        case 0x2210:  execute_rst(mem); break;

        // 23XX. Control Transfer Instructions - Returns

        case 0x2300:  execute_ret(mem, processor_condition_type::none);  break;
        case 0x2301:  execute_ret(mem, processor_condition_type::z);     break;
        case 0x2302:  execute_ret(mem, processor_condition_type::nz);    break;
        case 0x2303:  execute_ret(mem, processor_condition_type::c);     break;
        case 0x2304:  execute_ret(mem, processor_condition_type::nc);    break;
        case 0x2310:  execute_reti(mem); break;

        // 30XX. Arithmetic Instructions - Increments

        case 0x3000:  execute_inc_r8(processor_register_type::b0);        break;
        case 0x3001:  execute_inc_r8(processor_register_type::b1);        break;
        case 0x3002:  execute_inc_r8(processor_register_type::b2);        break;
        case 0x3003:  execute_inc_r8(processor_register_type::b3);        break;
        case 0x3004:  execute_inc_r8(processor_register_type::b4);        break;
        case 0x3005:  execute_inc_r8(processor_register_type::b5);        break;
        case 0x3006:  execute_inc_r8(processor_register_type::b6);        break;
        case 0x3007:  execute_inc_r8(processor_register_type::b7);        break;
        case 0x3008:  execute_inc_r8(processor_register_type::b8);        break;
        case 0x3009:  execute_inc_r8(processor_register_type::b9);        break;
        case 0x300A:  execute_inc_r8(processor_register_type::b10);       break;
        case 0x300B:  execute_inc_r8(processor_register_type::b11);       break;
        case 0x300C:  execute_inc_r8(processor_register_type::b12);       break;
        case 0x300D:  execute_inc_r8(processor_register_type::b13);       break;
        case 0x300E:  execute_inc_r8(processor_register_type::b14);       break;
        case 0x300F:  execute_inc_r8(processor_register_type::b15);       break;
        case 0x3010:  execute_inc_r16(processor_register_type::w0);       break;
        case 0x3011:  execute_inc_r16(processor_register_type::w1);       break;
        case 0x3012:  execute_inc_r16(processor_register_type::w2);       break;
        case 0x3013:  execute_inc_r16(processor_register_type::w3);       break;
        case 0x3014:  execute_inc_r16(processor_register_type::w4);       break;
        case 0x3015:  execute_inc_r16(processor_register_type::w5);       break;
        case 0x3016:  execute_inc_r16(processor_register_type::w6);       break;
        case 0x3017:  execute_inc_r16(processor_register_type::w7);       break;
        case 0x3018:  execute_inc_r32(processor_register_type::l0);       break;
        case 0x3019:  execute_inc_r32(processor_register_type::l1);       break;
        case 0x301A:  execute_inc_r32(processor_register_type::l2);       break;
        case 0x301B:  execute_inc_r32(processor_register_type::l3);       break;
        case 0x3020:  execute_inc_a32(mem);                               break;
        case 0x3030:  execute_inc_ar32(mem, processor_register_type::l0); break;
        case 0x3031:  execute_inc_ar32(mem, processor_register_type::l1); break;
        case 0x3032:  execute_inc_ar32(mem, processor_register_type::l2); break;
        case 0x3033:  execute_inc_ar32(mem, processor_register_type::l3); break;

        // 31XX. Arithmetic Instructions - Decrements

        case 0x3100:  execute_dec_r8(processor_register_type::b0);        break;
        case 0x3101:  execute_dec_r8(processor_register_type::b1);        break;
        case 0x3102:  execute_dec_r8(processor_register_type::b2);        break;
        case 0x3103:  execute_dec_r8(processor_register_type::b3);        break;
        case 0x3104:  execute_dec_r8(processor_register_type::b4);        break;
        case 0x3105:  execute_dec_r8(processor_register_type::b5);        break;
        case 0x3106:  execute_dec_r8(processor_register_type::b6);        break;
        case 0x3107:  execute_dec_r8(processor_register_type::b7);        break;
        case 0x3108:  execute_dec_r8(processor_register_type::b8);        break;
        case 0x3109:  execute_dec_r8(processor_register_type::b9);        break;
        case 0x310A:  execute_dec_r8(processor_register_type::b10);       break;
        case 0x310B:  execute_dec_r8(processor_register_type::b11);       break;
        case 0x310C:  execute_dec_r8(processor_register_type::b12);       break;
        case 0x310D:  execute_dec_r8(processor_register_type::b13);       break;
        case 0x310E:  execute_dec_r8(processor_register_type::b14);       break;
        case 0x310F:  execute_dec_r8(processor_register_type::b15);       break;
        case 0x3110:  execute_dec_r16(processor_register_type::w0);       break;
        case 0x3111:  execute_dec_r16(processor_register_type::w1);       break;
        case 0x3112:  execute_dec_r16(processor_register_type::w2);       break;
        case 0x3113:  execute_dec_r16(processor_register_type::w3);       break;
        case 0x3114:  execute_dec_r16(processor_register_type::w4);       break;
        case 0x3115:  execute_dec_r16(processor_register_type::w5);       break;
        case 0x3116:  execute_dec_r16(processor_register_type::w6);       break;
        case 0x3117:  execute_dec_r16(processor_register_type::w7);       break;
        case 0x3118:  execute_dec_r32(processor_register_type::l0);       break;
        case 0x3119:  execute_dec_r32(processor_register_type::l1);       break;
        case 0x311A:  execute_dec_r32(processor_register_type::l2);       break;
        case 0x311B:  execute_dec_r32(processor_register_type::l3);       break;
        case 0x3120:  execute_dec_a32(mem);                               break;
        case 0x3130:  execute_dec_ar32(mem, processor_register_type::l0); break;
        case 0x3131:  execute_dec_ar32(mem, processor_register_type::l1); break;
        case 0x3132:  execute_dec_ar32(mem, processor_register_type::l2); break;
        case 0x3133:  execute_dec_ar32(mem, processor_register_type::l3); break;

        // 32XX. Arithmetic Instructions - Addition

        case 0x3200:  execute_add_i8(mem);                                break;
        case 0x3210:  execute_add_r8(processor_register_type::b0);        break;
        case 0x3211:  execute_add_r8(processor_register_type::b1);        break;
        case 0x3212:  execute_add_r8(processor_register_type::b2);        break;
        case 0x3213:  execute_add_r8(processor_register_type::b3);        break;
        case 0x3214:  execute_add_r8(processor_register_type::b4);        break;
        case 0x3215:  execute_add_r8(processor_register_type::b5);        break;
        case 0x3216:  execute_add_r8(processor_register_type::b6);        break;
        case 0x3217:  execute_add_r8(processor_register_type::b7);        break;
        case 0x3218:  execute_add_r8(processor_register_type::b8);        break;
        case 0x3219:  execute_add_r8(processor_register_type::b9);        break;
        case 0x321A:  execute_add_r8(processor_register_type::b10);       break;
        case 0x321B:  execute_add_r8(processor_register_type::b11);       break;
        case 0x321C:  execute_add_r8(processor_register_type::b12);       break;
        case 0x321D:  execute_add_r8(processor_register_type::b13);       break;
        case 0x321E:  execute_add_r8(processor_register_type::b14);       break;
        case 0x321F:  execute_add_r8(processor_register_type::b15);       break;
        case 0x3220:  execute_add_a32(mem);                               break;
        case 0x3230:  execute_add_ar32(mem, processor_register_type::l0); break;
        case 0x3231:  execute_add_ar32(mem, processor_register_type::l1); break;
        case 0x3232:  execute_add_ar32(mem, processor_register_type::l2); break;
        case 0x3233:  execute_add_ar32(mem, processor_register_type::l3); break;

        case 0x3240:  execute_adc_i8(mem);                                break;
        case 0x3250:  execute_adc_r8(processor_register_type::b0);        break;
        case 0x3251:  execute_adc_r8(processor_register_type::b1);        break;
        case 0x3252:  execute_adc_r8(processor_register_type::b2);        break;
        case 0x3253:  execute_adc_r8(processor_register_type::b3);        break;
        case 0x3254:  execute_adc_r8(processor_register_type::b4);        break;
        case 0x3255:  execute_adc_r8(processor_register_type::b5);        break;
        case 0x3256:  execute_adc_r8(processor_register_type::b6);        break;
        case 0x3257:  execute_adc_r8(processor_register_type::b7);        break;
        case 0x3258:  execute_adc_r8(processor_register_type::b8);        break;
        case 0x3259:  execute_adc_r8(processor_register_type::b9);        break;
        case 0x325A:  execute_adc_r8(processor_register_type::b10);       break;
        case 0x325B:  execute_adc_r8(processor_register_type::b11);       break;
        case 0x325C:  execute_adc_r8(processor_register_type::b12);       break;
        case 0x325D:  execute_adc_r8(processor_register_type::b13);       break;
        case 0x325E:  execute_adc_r8(processor_register_type::b14);       break;
        case 0x325F:  execute_adc_r8(processor_register_type::b15);       break;
        case 0x3260:  execute_adc_a32(mem);                               break;
        case 0x3270:  execute_adc_ar32(mem, processor_register_type::l0); break;
        case 0x3271:  execute_adc_ar32(mem, processor_register_type::l1); break;
        case 0x3272:  execute_adc_ar32(mem, processor_register_type::l2); break;
        case 0x3273:  execute_adc_ar32(mem, processor_register_type::l3); break;

        // 33XX. Arithmetic Instructions - Subtraction

        case 0x3300:  execute_sub_i8(mem);                                break;
        case 0x3310:  execute_sub_r8(processor_register_type::b0);        break;
        case 0x3311:  execute_sub_r8(processor_register_type::b1);        break;
        case 0x3312:  execute_sub_r8(processor_register_type::b2);        break;
        case 0x3313:  execute_sub_r8(processor_register_type::b3);        break;
        case 0x3314:  execute_sub_r8(processor_register_type::b4);        break;
        case 0x3315:  execute_sub_r8(processor_register_type::b5);        break;
        case 0x3316:  execute_sub_r8(processor_register_type::b6);        break;
        case 0x3317:  execute_sub_r8(processor_register_type::b7);        break;
        case 0x3318:  execute_sub_r8(processor_register_type::b8);        break;
        case 0x3319:  execute_sub_r8(processor_register_type::b9);        break;
        case 0x331A:  execute_sub_r8(processor_register_type::b10);       break;
        case 0x331B:  execute_sub_r8(processor_register_type::b11);       break;
        case 0x331C:  execute_sub_r8(processor_register_type::b12);       break;
        case 0x331D:  execute_sub_r8(processor_register_type::b13);       break;
        case 0x331E:  execute_sub_r8(processor_register_type::b14);       break;
        case 0x331F:  execute_sub_r8(processor_register_type::b15);       break;
        case 0x3320:  execute_sub_a32(mem);                               break;
        case 0x3330:  execute_sub_ar32(mem, processor_register_type::l0); break;
        case 0x3331:  execute_sub_ar32(mem, processor_register_type::l1); break;
        case 0x3332:  execute_sub_ar32(mem, processor_register_type::l2); break;
        case 0x3333:  execute_sub_ar32(mem, processor_register_type::l3); break;

        case 0x3340:  execute_sbc_i8(mem);                                break;
        case 0x3350:  execute_sbc_r8(processor_register_type::b0);        break;
        case 0x3351:  execute_sbc_r8(processor_register_type::b1);        break;
        case 0x3352:  execute_sbc_r8(processor_register_type::b2);        break;
        case 0x3353:  execute_sbc_r8(processor_register_type::b3);        break;
        case 0x3354:  execute_sbc_r8(processor_register_type::b4);        break;
        case 0x3355:  execute_sbc_r8(processor_register_type::b5);        break;
        case 0x3356:  execute_sbc_r8(processor_register_type::b6);        break;
        case 0x3357:  execute_sbc_r8(processor_register_type::b7);        break;
        case 0x3358:  execute_sbc_r8(processor_register_type::b8);        break;
        case 0x3359:  execute_sbc_r8(processor_register_type::b9);        break;
        case 0x335A:  execute_sbc_r8(processor_register_type::b10);       break;
        case 0x335B:  execute_sbc_r8(processor_register_type::b11);       break;
        case 0x335C:  execute_sbc_r8(processor_register_type::b12);       break;
        case 0x335D:  execute_sbc_r8(processor_register_type::b13);       break;
        case 0x335E:  execute_sbc_r8(processor_register_type::b14);       break;
        case 0x335F:  execute_sbc_r8(processor_register_type::b15);       break;
        case 0x3360:  execute_sbc_a32(mem);                               break;
        case 0x3370:  execute_sbc_ar32(mem, processor_register_type::l0); break;
        case 0x3371:  execute_sbc_ar32(mem, processor_register_type::l1); break;
        case 0x3372:  execute_sbc_ar32(mem, processor_register_type::l2); break;
        case 0x3373:  execute_sbc_ar32(mem, processor_register_type::l3); break;

        // 50XX. Logic Instructions - AND

        case 0x5000:  execute_and_i8(mem);                                break;
        case 0x5010:  execute_and_r8(processor_register_type::b0);        break;
        case 0x5011:  execute_and_r8(processor_register_type::b1);        break;
        case 0x5012:  execute_and_r8(processor_register_type::b2);        break;
        case 0x5013:  execute_and_r8(processor_register_type::b3);        break;
        case 0x5014:  execute_and_r8(processor_register_type::b4);        break;
        case 0x5015:  execute_and_r8(processor_register_type::b5);        break;
        case 0x5016:  execute_and_r8(processor_register_type::b6);        break;
        case 0x5017:  execute_and_r8(processor_register_type::b7);        break;
        case 0x5018:  execute_and_r8(processor_register_type::b8);        break;
        case 0x5019:  execute_and_r8(processor_register_type::b9);        break;
        case 0x501A:  execute_and_r8(processor_register_type::b10);       break;
        case 0x501B:  execute_and_r8(processor_register_type::b11);       break;
        case 0x501C:  execute_and_r8(processor_register_type::b12);       break;
        case 0x501D:  execute_and_r8(processor_register_type::b13);       break;
        case 0x501E:  execute_and_r8(processor_register_type::b14);       break;
        case 0x501F:  execute_and_r8(processor_register_type::b15);       break;
        case 0x5020:  execute_and_a32(mem);                               break;
        case 0x5030:  execute_and_ar32(mem, processor_register_type::l0); break;
        case 0x5031:  execute_and_ar32(mem, processor_register_type::l1); break;
        case 0x5032:  execute_and_ar32(mem, processor_register_type::l2); break;
        case 0x5033:  execute_and_ar32(mem, processor_register_type::l3); break;

        // 51XX. Logic Instructions - OR

        case 0x5100:  execute_or_i8(mem);                                 break;
        case 0x5110:  execute_or_r8(processor_register_type::b0);         break;
        case 0x5111:  execute_or_r8(processor_register_type::b1);         break;
        case 0x5112:  execute_or_r8(processor_register_type::b2);         break;
        case 0x5113:  execute_or_r8(processor_register_type::b3);         break;
        case 0x5114:  execute_or_r8(processor_register_type::b4);         break;
        case 0x5115:  execute_or_r8(processor_register_type::b5);         break;
        case 0x5116:  execute_or_r8(processor_register_type::b6);         break;
        case 0x5117:  execute_or_r8(processor_register_type::b7);         break;
        case 0x5118:  execute_or_r8(processor_register_type::b8);         break;
        case 0x5119:  execute_or_r8(processor_register_type::b9);         break;
        case 0x511A:  execute_or_r8(processor_register_type::b10);        break;
        case 0x511B:  execute_or_r8(processor_register_type::b11);        break;
        case 0x511C:  execute_or_r8(processor_register_type::b12);        break;
        case 0x511D:  execute_or_r8(processor_register_type::b13);        break;
        case 0x511E:  execute_or_r8(processor_register_type::b14);        break;
        case 0x511F:  execute_or_r8(processor_register_type::b15);        break;
        case 0x5120:  execute_or_a32(mem);                                break;
        case 0x5130:  execute_or_ar32(mem, processor_register_type::l0);  break;
        case 0x5131:  execute_or_ar32(mem, processor_register_type::l1);  break;
        case 0x5132:  execute_or_ar32(mem, processor_register_type::l2);  break;
        case 0x5133:  execute_or_ar32(mem, processor_register_type::l3);  break;

        // 52XX. Logic Instructions - XOR

        case 0x5200:  execute_xor_i8(mem);                                break;
        case 0x5210:  execute_xor_r8(processor_register_type::b0);        break;
        case 0x5211:  execute_xor_r8(processor_register_type::b1);        break;
        case 0x5212:  execute_xor_r8(processor_register_type::b2);        break;
        case 0x5213:  execute_xor_r8(processor_register_type::b3);        break;
        case 0x5214:  execute_xor_r8(processor_register_type::b4);        break;
        case 0x5215:  execute_xor_r8(processor_register_type::b5);        break;
        case 0x5216:  execute_xor_r8(processor_register_type::b6);        break;
        case 0x5217:  execute_xor_r8(processor_register_type::b7);        break;
        case 0x5218:  execute_xor_r8(processor_register_type::b8);        break;
        case 0x5219:  execute_xor_r8(processor_register_type::b9);        break;
        case 0x521A:  execute_xor_r8(processor_register_type::b10);       break;
        case 0x521B:  execute_xor_r8(processor_register_type::b11);       break;
        case 0x521C:  execute_xor_r8(processor_register_type::b12);       break;
        case 0x521D:  execute_xor_r8(processor_register_type::b13);       break;
        case 0x521E:  execute_xor_r8(processor_register_type::b14);       break;
        case 0x521F:  execute_xor_r8(processor_register_type::b15);       break;
        case 0x5220:  execute_xor_a32(mem);                               break;
        case 0x5230:  execute_xor_ar32(mem, processor_register_type::l0); break;
        case 0x5231:  execute_xor_ar32(mem, processor_register_type::l1); break;
        case 0x5232:  execute_xor_ar32(mem, processor_register_type::l2); break;
        case 0x5233:  execute_xor_ar32(mem, processor_register_type::l3); break;

        // 53XX. Logic Instructions - CMP

        case 0x5300:  execute_cmp_i8(mem);                                break;
        case 0x5310:  execute_cmp_r8(processor_register_type::b0);        break;
        case 0x5311:  execute_cmp_r8(processor_register_type::b1);        break;
        case 0x5312:  execute_cmp_r8(processor_register_type::b2);        break;
        case 0x5313:  execute_cmp_r8(processor_register_type::b3);        break;
        case 0x5314:  execute_cmp_r8(processor_register_type::b4);        break;
        case 0x5315:  execute_cmp_r8(processor_register_type::b5);        break;
        case 0x5316:  execute_cmp_r8(processor_register_type::b6);        break;
        case 0x5317:  execute_cmp_r8(processor_register_type::b7);        break;
        case 0x5318:  execute_cmp_r8(processor_register_type::b8);        break;
        case 0x5319:  execute_cmp_r8(processor_register_type::b9);        break;
        case 0x531A:  execute_cmp_r8(processor_register_type::b10);       break;
        case 0x531B:  execute_cmp_r8(processor_register_type::b11);       break;
        case 0x531C:  execute_cmp_r8(processor_register_type::b12);       break;
        case 0x531D:  execute_cmp_r8(processor_register_type::b13);       break;
        case 0x531E:  execute_cmp_r8(processor_register_type::b14);       break;
        case 0x531F:  execute_cmp_r8(processor_register_type::b15);       break;
        case 0x5320:  execute_cmp_a32(mem);                               break;
        case 0x5330:  execute_cmp_ar32(mem, processor_register_type::l0); break;
        case 0x5331:  execute_cmp_ar32(mem, processor_register_type::l1); break;
        case 0x5332:  execute_cmp_ar32(mem, processor_register_type::l2); break;
        case 0x5333:  execute_cmp_ar32(mem, processor_register_type::l3); break;

        // 60XX. Bitwise Instructions - BIT
        case 0x6010:  execute_bit_r8(mem, processor_register_type::b0);   break;
        case 0x6011:  execute_bit_r8(mem, processor_register_type::b1);   break;
        case 0x6012:  execute_bit_r8(mem, processor_register_type::b2);   break;
        case 0x6013:  execute_bit_r8(mem, processor_register_type::b3);   break;
        case 0x6014:  execute_bit_r8(mem, processor_register_type::b4);   break;
        case 0x6015:  execute_bit_r8(mem, processor_register_type::b5);   break;
        case 0x6016:  execute_bit_r8(mem, processor_register_type::b6);   break;
        case 0x6017:  execute_bit_r8(mem, processor_register_type::b7);   break;
        case 0x6018:  execute_bit_r8(mem, processor_register_type::b8);   break;
        case 0x6019:  execute_bit_r8(mem, processor_register_type::b9);   break;
        case 0x601A:  execute_bit_r8(mem, processor_register_type::b10);  break;
        case 0x601B:  execute_bit_r8(mem, processor_register_type::b11);  break;
        case 0x601C:  execute_bit_r8(mem, processor_register_type::b12);  break;
        case 0x601D:  execute_bit_r8(mem, processor_register_type::b13);  break;
        case 0x601E:  execute_bit_r8(mem, processor_register_type::b14);  break;
        case 0x601F:  execute_bit_r8(mem, processor_register_type::b15);  break;
        case 0x6020:  execute_bit_a32(mem);                               break;
        case 0x6030:  execute_bit_ar32(mem, processor_register_type::l0); break;
        case 0x6031:  execute_bit_ar32(mem, processor_register_type::l1); break;
        case 0x6032:  execute_bit_ar32(mem, processor_register_type::l2); break;
        case 0x6033:  execute_bit_ar32(mem, processor_register_type::l3); break;

        // 61XX. Bitwise Instructions - SET
        case 0x6110:  execute_set_r8(mem, processor_register_type::b0);   break;
        case 0x6111:  execute_set_r8(mem, processor_register_type::b1);   break;
        case 0x6112:  execute_set_r8(mem, processor_register_type::b2);   break;
        case 0x6113:  execute_set_r8(mem, processor_register_type::b3);   break;
        case 0x6114:  execute_set_r8(mem, processor_register_type::b4);   break;
        case 0x6115:  execute_set_r8(mem, processor_register_type::b5);   break;
        case 0x6116:  execute_set_r8(mem, processor_register_type::b6);   break;
        case 0x6117:  execute_set_r8(mem, processor_register_type::b7);   break;
        case 0x6118:  execute_set_r8(mem, processor_register_type::b8);   break;
        case 0x6119:  execute_set_r8(mem, processor_register_type::b9);   break;
        case 0x611A:  execute_set_r8(mem, processor_register_type::b10);  break;
        case 0x611B:  execute_set_r8(mem, processor_register_type::b11);  break;
        case 0x611C:  execute_set_r8(mem, processor_register_type::b12);  break;
        case 0x611D:  execute_set_r8(mem, processor_register_type::b13);  break;
        case 0x611E:  execute_set_r8(mem, processor_register_type::b14);  break;
        case 0x611F:  execute_set_r8(mem, processor_register_type::b15);  break;
        case 0x6120:  execute_set_a32(mem);                               break;
        case 0x6130:  execute_set_ar32(mem, processor_register_type::l0); break;
        case 0x6131:  execute_set_ar32(mem, processor_register_type::l1); break;
        case 0x6132:  execute_set_ar32(mem, processor_register_type::l2); break;
        case 0x6133:  execute_set_ar32(mem, processor_register_type::l3); break;

        // 62XX. Bitwise Instructions - RES
        case 0x6210:  execute_res_r8(mem, processor_register_type::b0);   break;
        case 0x6211:  execute_res_r8(mem, processor_register_type::b1);   break;
        case 0x6212:  execute_res_r8(mem, processor_register_type::b2);   break;
        case 0x6213:  execute_res_r8(mem, processor_register_type::b3);   break;
        case 0x6214:  execute_res_r8(mem, processor_register_type::b4);   break;
        case 0x6215:  execute_res_r8(mem, processor_register_type::b5);   break;
        case 0x6216:  execute_res_r8(mem, processor_register_type::b6);   break;
        case 0x6217:  execute_res_r8(mem, processor_register_type::b7);   break;
        case 0x6218:  execute_res_r8(mem, processor_register_type::b8);   break;
        case 0x6219:  execute_res_r8(mem, processor_register_type::b9);   break;
        case 0x621A:  execute_res_r8(mem, processor_register_type::b10);  break;
        case 0x621B:  execute_res_r8(mem, processor_register_type::b11);  break;
        case 0x621C:  execute_res_r8(mem, processor_register_type::b12);  break;
        case 0x621D:  execute_res_r8(mem, processor_register_type::b13);  break;
        case 0x621E:  execute_res_r8(mem, processor_register_type::b14);  break;
        case 0x621F:  execute_res_r8(mem, processor_register_type::b15);  break;
        case 0x6220:  execute_res_a32(mem);                               break;
        case 0x6230:  execute_res_ar32(mem, processor_register_type::l0); break;
        case 0x6231:  execute_res_ar32(mem, processor_register_type::l1); break;
        case 0x6232:  execute_res_ar32(mem, processor_register_type::l2); break;
        case 0x6233:  execute_res_ar32(mem, processor_register_type::l3); break;

        // 70XX. Shift and Rotate Instructions - SLA
        case 0x7010:  execute_sla_r8(processor_register_type::b0);        break;
        case 0x7011:  execute_sla_r8(processor_register_type::b1);        break;
        case 0x7012:  execute_sla_r8(processor_register_type::b2);        break;
        case 0x7013:  execute_sla_r8(processor_register_type::b3);        break;
        case 0x7014:  execute_sla_r8(processor_register_type::b4);        break;
        case 0x7015:  execute_sla_r8(processor_register_type::b5);        break;
        case 0x7016:  execute_sla_r8(processor_register_type::b6);        break;
        case 0x7017:  execute_sla_r8(processor_register_type::b7);        break;
        case 0x7018:  execute_sla_r8(processor_register_type::b8);        break;
        case 0x7019:  execute_sla_r8(processor_register_type::b9);        break;
        case 0x701A:  execute_sla_r8(processor_register_type::b10);       break;
        case 0x701B:  execute_sla_r8(processor_register_type::b11);       break;
        case 0x701C:  execute_sla_r8(processor_register_type::b12);       break;
        case 0x701D:  execute_sla_r8(processor_register_type::b13);       break;
        case 0x701E:  execute_sla_r8(processor_register_type::b14);       break;
        case 0x701F:  execute_sla_r8(processor_register_type::b15);       break;
        case 0x7020:  execute_sla_a32(mem);                               break;
        case 0x7030:  execute_sla_ar32(mem, processor_register_type::l0); break;
        case 0x7031:  execute_sla_ar32(mem, processor_register_type::l1); break;
        case 0x7032:  execute_sla_ar32(mem, processor_register_type::l2); break;
        case 0x7033:  execute_sla_ar32(mem, processor_register_type::l3); break;

        // 71XX. Shift and Rotate Instructions - SRA
        case 0x7110:  execute_sra_r8(processor_register_type::b0);        break;
        case 0x7111:  execute_sra_r8(processor_register_type::b1);        break;
        case 0x7112:  execute_sra_r8(processor_register_type::b2);        break;
        case 0x7113:  execute_sra_r8(processor_register_type::b3);        break;
        case 0x7114:  execute_sra_r8(processor_register_type::b4);        break;
        case 0x7115:  execute_sra_r8(processor_register_type::b5);        break;
        case 0x7116:  execute_sra_r8(processor_register_type::b6);        break;
        case 0x7117:  execute_sra_r8(processor_register_type::b7);        break;
        case 0x7118:  execute_sra_r8(processor_register_type::b8);        break;
        case 0x7119:  execute_sra_r8(processor_register_type::b9);        break;
        case 0x711A:  execute_sra_r8(processor_register_type::b10);       break;
        case 0x711B:  execute_sra_r8(processor_register_type::b11);       break;
        case 0x711C:  execute_sra_r8(processor_register_type::b12);       break;
        case 0x711D:  execute_sra_r8(processor_register_type::b13);       break;
        case 0x711E:  execute_sra_r8(processor_register_type::b14);       break;
        case 0x711F:  execute_sra_r8(processor_register_type::b15);       break;
        case 0x7120:  execute_sra_a32(mem);                               break;
        case 0x7130:  execute_sra_ar32(mem, processor_register_type::l0); break;
        case 0x7131:  execute_sra_ar32(mem, processor_register_type::l1); break;
        case 0x7132:  execute_sra_ar32(mem, processor_register_type::l2); break;
        case 0x7133:  execute_sra_ar32(mem, processor_register_type::l3); break;

        // 72XX. Shift and Rotate Instructions - SRL
        case 0x7210:  execute_srl_r8(processor_register_type::b0);        break;
        case 0x7211:  execute_srl_r8(processor_register_type::b1);        break;
        case 0x7212:  execute_srl_r8(processor_register_type::b2);        break;
        case 0x7213:  execute_srl_r8(processor_register_type::b3);        break;
        case 0x7214:  execute_srl_r8(processor_register_type::b4);        break;
        case 0x7215:  execute_srl_r8(processor_register_type::b5);        break;
        case 0x7216:  execute_srl_r8(processor_register_type::b6);        break;
        case 0x7217:  execute_srl_r8(processor_register_type::b7);        break;
        case 0x7218:  execute_srl_r8(processor_register_type::b8);        break;
        case 0x7219:  execute_srl_r8(processor_register_type::b9);        break;
        case 0x721A:  execute_srl_r8(processor_register_type::b10);       break;
        case 0x721B:  execute_srl_r8(processor_register_type::b11);       break;
        case 0x721C:  execute_srl_r8(processor_register_type::b12);       break;
        case 0x721D:  execute_srl_r8(processor_register_type::b13);       break;
        case 0x721E:  execute_srl_r8(processor_register_type::b14);       break;
        case 0x721F:  execute_srl_r8(processor_register_type::b15);       break;
        case 0x7220:  execute_srl_a32(mem);                               break;
        case 0x7230:  execute_srl_ar32(mem, processor_register_type::l0); break;
        case 0x7231:  execute_srl_ar32(mem, processor_register_type::l1); break;
        case 0x7232:  execute_srl_ar32(mem, processor_register_type::l2); break;
        case 0x7233:  execute_srl_ar32(mem, processor_register_type::l3); break;

        // 73XX. Shift and Rotate Instructions - RL
        case 0x7310:  execute_rl_r8(processor_register_type::b0);         break;
        case 0x7311:  execute_rl_r8(processor_register_type::b1);         break;
        case 0x7312:  execute_rl_r8(processor_register_type::b2);         break;
        case 0x7313:  execute_rl_r8(processor_register_type::b3);         break;
        case 0x7314:  execute_rl_r8(processor_register_type::b4);         break;
        case 0x7315:  execute_rl_r8(processor_register_type::b5);         break;
        case 0x7316:  execute_rl_r8(processor_register_type::b6);         break;
        case 0x7317:  execute_rl_r8(processor_register_type::b7);         break;
        case 0x7318:  execute_rl_r8(processor_register_type::b8);         break;
        case 0x7319:  execute_rl_r8(processor_register_type::b9);         break;
        case 0x731A:  execute_rl_r8(processor_register_type::b10);        break;
        case 0x731B:  execute_rl_r8(processor_register_type::b11);        break;
        case 0x731C:  execute_rl_r8(processor_register_type::b12);        break;
        case 0x731D:  execute_rl_r8(processor_register_type::b13);        break;
        case 0x731E:  execute_rl_r8(processor_register_type::b14);        break;
        case 0x731F:  execute_rl_r8(processor_register_type::b15);        break;
        case 0x7320:  execute_rl_a32(mem);                                break;
        case 0x7330:  execute_rl_ar32(mem, processor_register_type::l0);  break;
        case 0x7331:  execute_rl_ar32(mem, processor_register_type::l1);  break;
        case 0x7332:  execute_rl_ar32(mem, processor_register_type::l2);  break;
        case 0x7333:  execute_rl_ar32(mem, processor_register_type::l3);  break;
        case 0x7340:  execute_rla();                                      break;

        // 74XX. Shift and Rotate Instructions - RLC
        case 0x7410:  execute_rlc_r8(processor_register_type::b0);        break;
        case 0x7411:  execute_rlc_r8(processor_register_type::b1);        break;
        case 0x7412:  execute_rlc_r8(processor_register_type::b2);        break;
        case 0x7413:  execute_rlc_r8(processor_register_type::b3);        break;
        case 0x7414:  execute_rlc_r8(processor_register_type::b4);        break;
        case 0x7415:  execute_rlc_r8(processor_register_type::b5);        break;
        case 0x7416:  execute_rlc_r8(processor_register_type::b6);        break;
        case 0x7417:  execute_rlc_r8(processor_register_type::b7);        break;
        case 0x7418:  execute_rlc_r8(processor_register_type::b8);        break;
        case 0x7419:  execute_rlc_r8(processor_register_type::b9);        break;
        case 0x741A:  execute_rlc_r8(processor_register_type::b10);       break;
        case 0x741B:  execute_rlc_r8(processor_register_type::b11);       break;
        case 0x741C:  execute_rlc_r8(processor_register_type::b12);       break;
        case 0x741D:  execute_rlc_r8(processor_register_type::b13);       break;
        case 0x741E:  execute_rlc_r8(processor_register_type::b14);       break;
        case 0x741F:  execute_rlc_r8(processor_register_type::b15);       break;
        case 0x7420:  execute_rlc_a32(mem);                               break;
        case 0x7430:  execute_rlc_ar32(mem, processor_register_type::l0); break;
        case 0x7431:  execute_rlc_ar32(mem, processor_register_type::l1); break;
        case 0x7432:  execute_rlc_ar32(mem, processor_register_type::l2); break;
        case 0x7433:  execute_rlc_ar32(mem, processor_register_type::l3); break;
        case 0x7440:  execute_rlca();                                     break;

        // 75XX. Shift and Rotate Instructions - RR
        case 0x7510:  execute_rr_r8(processor_register_type::b0);         break;
        case 0x7511:  execute_rr_r8(processor_register_type::b1);         break;
        case 0x7512:  execute_rr_r8(processor_register_type::b2);         break;
        case 0x7513:  execute_rr_r8(processor_register_type::b3);         break;
        case 0x7514:  execute_rr_r8(processor_register_type::b4);         break;
        case 0x7515:  execute_rr_r8(processor_register_type::b5);         break;
        case 0x7516:  execute_rr_r8(processor_register_type::b6);         break;
        case 0x7517:  execute_rr_r8(processor_register_type::b7);         break;
        case 0x7518:  execute_rr_r8(processor_register_type::b8);         break;
        case 0x7519:  execute_rr_r8(processor_register_type::b9);         break;
        case 0x751A:  execute_rr_r8(processor_register_type::b10);        break;
        case 0x751B:  execute_rr_r8(processor_register_type::b11);        break;
        case 0x751C:  execute_rr_r8(processor_register_type::b12);        break;
        case 0x751D:  execute_rr_r8(processor_register_type::b13);        break;
        case 0x751E:  execute_rr_r8(processor_register_type::b14);        break;
        case 0x751F:  execute_rr_r8(processor_register_type::b15);        break;
        case 0x7520:  execute_rr_a32(mem);                                break;
        case 0x7530:  execute_rr_ar32(mem, processor_register_type::l0);  break;
        case 0x7531:  execute_rr_ar32(mem, processor_register_type::l1);  break;
        case 0x7532:  execute_rr_ar32(mem, processor_register_type::l2);  break;
        case 0x7533:  execute_rr_ar32(mem, processor_register_type::l3);  break;
        case 0x7540:  execute_rra();                                      break;

        // 76XX. Shift and Rotate Instructions - RRC
        case 0x7610:  execute_rrc_r8(processor_register_type::b0);        break;
        case 0x7611:  execute_rrc_r8(processor_register_type::b1);        break;
        case 0x7612:  execute_rrc_r8(processor_register_type::b2);        break;
        case 0x7613:  execute_rrc_r8(processor_register_type::b3);        break;
        case 0x7614:  execute_rrc_r8(processor_register_type::b4);        break;
        case 0x7615:  execute_rrc_r8(processor_register_type::b5);        break;
        case 0x7616:  execute_rrc_r8(processor_register_type::b6);        break;
        case 0x7617:  execute_rrc_r8(processor_register_type::b7);        break;
        case 0x7618:  execute_rrc_r8(processor_register_type::b8);        break;
        case 0x7619:  execute_rrc_r8(processor_register_type::b9);        break;
        case 0x761A:  execute_rrc_r8(processor_register_type::b10);       break;
        case 0x761B:  execute_rrc_r8(processor_register_type::b11);       break;
        case 0x761C:  execute_rrc_r8(processor_register_type::b12);       break;
        case 0x761D:  execute_rrc_r8(processor_register_type::b13);       break;
        case 0x761E:  execute_rrc_r8(processor_register_type::b14);       break;
        case 0x761F:  execute_rrc_r8(processor_register_type::b15);       break;
        case 0x7620:  execute_rrc_a32(mem);                               break;
        case 0x7630:  execute_rrc_ar32(mem, processor_register_type::l0); break;
        case 0x7631:  execute_rrc_ar32(mem, processor_register_type::l1); break;
        case 0x7632:  execute_rrc_ar32(mem, processor_register_type::l2); break;
        case 0x7633:  execute_rrc_ar32(mem, processor_register_type::l3); break;
        case 0x7640:  execute_rrca();                                     break;

        case 0xFFFF:  execute_rst0();                                     break;

        // Invalid Opcode
        default:
          std::cerr <<  "[processor::step] "
                    <<  "Invalid operation code: " << opcode << "!"
                    <<  std::endl;
          throw std::runtime_error { "[processor::step] Invalid operation code!" };

      }
    } else {
      cycle(1);

      if (m_interrupts_requested != 0) {
        set_flag(processor_flag_type::halt, true);
      }
    }

    // Handle CPU interrupts if they are currently enabled.
    if (check_flag(processor_flag_type::interrupt_disable) == false) {
      handle_interrupts(mem);
      set_flag(processor_flag_type::interrupt_enable, false);
    }

    // Clear the interrupt disable flag if the interrupt enable flag is set.
    if (check_flag(processor_flag_type::interrupt_enable) == true) {
      set_flag(processor_flag_type::interrupt_disable, false);
    }

  }

  std::uint32_t processor::read_register (const processor_register_type& type) const
  {
    switch (type)
    {

      // Direct 8-Bit Registers
      case processor_register_type::b0: return m_registers[0] & 0xFF;
      case processor_register_type::b1: return m_registers[1] & 0xFF;
      case processor_register_type::b2: return m_registers[2] & 0xFF;
      case processor_register_type::b3: return m_registers[3] & 0xFF;
      case processor_register_type::b4: return m_registers[4] & 0xFF;
      case processor_register_type::b5: return m_registers[5] & 0xFF;
      case processor_register_type::b6: return m_registers[6] & 0xFF;
      case processor_register_type::b7: return m_registers[7] & 0xFF;
      case processor_register_type::b8: return m_registers[8] & 0xFF;
      case processor_register_type::b9: return m_registers[9] & 0xFF;
      case processor_register_type::b10: return m_registers[10] & 0xFF;
      case processor_register_type::b11: return m_registers[11] & 0xFF;
      case processor_register_type::b12: return m_registers[12] & 0xFF;
      case processor_register_type::b13: return m_registers[13] & 0xFF;
      case processor_register_type::b14: return m_registers[14] & 0xFF;
      case processor_register_type::b15: return m_registers[15] & 0xFF;

      // Indirect 16-Bit Registers
      case processor_register_type::w0: return (m_registers[0] << 8) | m_registers[1];
      case processor_register_type::w1: return (m_registers[2] << 8) | m_registers[3];
      case processor_register_type::w2: return (m_registers[4] << 8) | m_registers[5];
      case processor_register_type::w3: return (m_registers[6] << 8) | m_registers[7];
      case processor_register_type::w4: return (m_registers[8] << 8) | m_registers[9];
      case processor_register_type::w5: return (m_registers[10] << 8) | m_registers[11];
      case processor_register_type::w6: return (m_registers[12] << 8) | m_registers[13];
      case processor_register_type::w7: return (m_registers[14] << 8) | m_registers[15];

      // Indirect 32-Bit Registers
      case processor_register_type::l0:
        return  (m_registers[0] << 24) | 
                (m_registers[1] << 16) | 
                (m_registers[2] << 8)  | 
                 m_registers[3];
      case processor_register_type::l1:
        return  (m_registers[4] << 24) | 
                (m_registers[5] << 16) | 
                (m_registers[6] << 8)  | 
                 m_registers[7];
      case processor_register_type::l2:
        return  (m_registers[8] << 24) | 
                (m_registers[9] << 16) | 
                (m_registers[10] << 8)  | 
                 m_registers[11];
      case processor_register_type::l3:
        return  (m_registers[12] << 24) | 
                (m_registers[13] << 16) | 
                (m_registers[14] << 8)  | 
                 m_registers[15];

      default:
        return 0xFFFFFFFF;

    }
  }

  bool processor::check_flag (const processor_flag_type& type) const
  {
    switch (type)
    {
      case processor_flag_type::zero:               return sm_getbit(m_flags, 7);
      case processor_flag_type::negative:           return sm_getbit(m_flags, 6);
      case processor_flag_type::half_carry:         return sm_getbit(m_flags, 5);
      case processor_flag_type::carry:              return sm_getbit(m_flags, 4);
      case processor_flag_type::interrupt_disable:  return sm_getbit(m_flags, 3);
      case processor_flag_type::interrupt_enable:   return sm_getbit(m_flags, 2);
      case processor_flag_type::halt:               return sm_getbit(m_flags, 1);
      case processor_flag_type::stop:               return sm_getbit(m_flags, 0);
      default:                                      return false;
    }
  }

  void processor::write_register (const processor_register_type& type, std::uint32_t value)
  {
    switch (type)
    {
      case processor_register_type::b0:   m_registers[0]  = (value & 0xFF); break;
      case processor_register_type::b1:   m_registers[1]  = (value & 0xFF); break;
      case processor_register_type::b2:   m_registers[2]  = (value & 0xFF); break;
      case processor_register_type::b3:   m_registers[3]  = (value & 0xFF); break;
      case processor_register_type::b4:   m_registers[4]  = (value & 0xFF); break;
      case processor_register_type::b5:   m_registers[5]  = (value & 0xFF); break;
      case processor_register_type::b6:   m_registers[6]  = (value & 0xFF); break;
      case processor_register_type::b7:   m_registers[7]  = (value & 0xFF); break;
      case processor_register_type::b8:   m_registers[8]  = (value & 0xFF); break;
      case processor_register_type::b9:   m_registers[9]  = (value & 0xFF); break;
      case processor_register_type::b10:  m_registers[10] = (value & 0xFF); break;
      case processor_register_type::b11:  m_registers[11] = (value & 0xFF); break;
      case processor_register_type::b12:  m_registers[12] = (value & 0xFF); break;
      case processor_register_type::b13:  m_registers[13] = (value & 0xFF); break;
      case processor_register_type::b14:  m_registers[14] = (value & 0xFF); break;
      case processor_register_type::b15:  m_registers[15] = (value & 0xFF); break;
      case processor_register_type::w0:
        m_registers[0] = ((value >> 8) & 0xFF);
        m_registers[1] = ((value)      & 0xFF);
        break;
      case processor_register_type::w1:
        m_registers[2] = ((value >> 8) & 0xFF);
        m_registers[3] = ((value)      & 0xFF);
        break;
      case processor_register_type::w2:
        m_registers[4] = ((value >> 8) & 0xFF);
        m_registers[5] = ((value)      & 0xFF);
        break;
      case processor_register_type::w3:
        m_registers[6] = ((value >> 8) & 0xFF);
        m_registers[7] = ((value)      & 0xFF);
        break;
      case processor_register_type::w4:
        m_registers[8] = ((value >> 8) & 0xFF);
        m_registers[9] = ((value)      & 0xFF);
        break;
      case processor_register_type::w5:
        m_registers[10] = ((value >> 8) & 0xFF);
        m_registers[11] = ((value)      & 0xFF);
        break;
      case processor_register_type::w6:
        m_registers[12] = ((value >> 8) & 0xFF);
        m_registers[13] = ((value)      & 0xFF);
        break;
      case processor_register_type::w7:
        m_registers[14] = ((value >> 8) & 0xFF);
        m_registers[15] = ((value)      & 0xFF);
        break;
      case processor_register_type::l0:
        m_registers[0]  = ((value >> 24) & 0xFF);
        m_registers[1]  = ((value >> 16) & 0xFF);
        m_registers[2]  = ((value >>  8) & 0xFF);
        m_registers[3]  = ((value)       & 0xFF);
        break;
      case processor_register_type::l1:
        m_registers[4]  = ((value >> 24) & 0xFF);
        m_registers[5]  = ((value >> 16) & 0xFF);
        m_registers[6]  = ((value >>  8) & 0xFF);
        m_registers[7]  = ((value)       & 0xFF);
        break;
      case processor_register_type::l2:
        m_registers[8]  = ((value >> 24) & 0xFF);
        m_registers[9]  = ((value >> 16) & 0xFF);
        m_registers[10] = ((value >>  8) & 0xFF);
        m_registers[11] = ((value)       & 0xFF);
        break;
      case processor_register_type::l3:
        m_registers[12] = ((value >> 24) & 0xFF);
        m_registers[13] = ((value >> 16) & 0xFF);
        m_registers[14] = ((value >>  8) & 0xFF);
        m_registers[15] = ((value)       & 0xFF);
        break;
    }
  }

  void processor::set_flag (const processor_flag_type& type, bool on)
  {
    switch (type)
    {
      case processor_flag_type::zero:               sm_setbit(m_flags, 7, on); break;
      case processor_flag_type::negative:           sm_setbit(m_flags, 6, on); break;
      case processor_flag_type::half_carry:         sm_setbit(m_flags, 5, on); break;
      case processor_flag_type::carry:              sm_setbit(m_flags, 4, on); break;
      case processor_flag_type::interrupt_disable:  sm_setbit(m_flags, 3, on); break;
      case processor_flag_type::interrupt_enable:   sm_setbit(m_flags, 2, on); break;
      case processor_flag_type::halt:               sm_setbit(m_flags, 1, on); break;
      case processor_flag_type::stop:               sm_setbit(m_flags, 0, on); break;
    }
  }

  /** Private Methods *****************************************************************************/

  void processor::advance (std::uint32_t count)
  {
    cycle(count);
    m_program_counter += count;
  }

  bool processor::check_condition (const processor_condition_type& cond) const
  {
    switch (cond)
    {
      case processor_condition_type::none:     return true;
      case processor_condition_type::zero:     return check_flag(processor_flag_type::zero) == true;
      case processor_condition_type::no_zero:  return check_flag(processor_flag_type::zero) == false;
      case processor_condition_type::carry:    return check_flag(processor_flag_type::carry) == true;
      case processor_condition_type::no_carry: return check_flag(processor_flag_type::carry) == false;
      default: return false;
    }
  }

  bool processor::check_interrupt (memory& mem, std::uint8_t id)
  {
    if (
      sm_getbit(m_interrupts_enabled, id) &&
      sm_getbit(m_interrupts_requested, id)
    ) {
      mem.push_long(m_stack_pointer, m_program_counter);
      m_program_counter = 0x80 + (0x10 * id);

      sm_setbit(m_interrupts_requested, id, false);
      set_flag(processor_flag_type::halt, false);
      set_flag(processor_flag_type::interrupt_disable, true);

      return true;
    }

    return false;
  }

  void processor::handle_interrupts (memory& mem)
  {
    if      (check_interrupt(mem, 0) == true) {}
    else if (check_interrupt(mem, 1) == true) {}
    else if (check_interrupt(mem, 2) == true) {}
    else if (check_interrupt(mem, 3) == true) {}
    else if (check_interrupt(mem, 4) == true) {}
    else if (check_interrupt(mem, 5) == true) {}
    else if (check_interrupt(mem, 6) == true) {}
    else if (check_interrupt(mem, 7) == true) {}
  }

  /** Instruction Execution Methods ***************************************************************/
  /** 0x0XXX. General Instructions ****************************************************************/

  void processor::execute_nop ()
  {

  }

  void processor::execute_halt ()
  {
    set_flag(processor_flag_type::halt, true);
  }

  void processor::execute_stop ()
  {
    set_flag(processor_flag_type::stop, true);
  }

  void processor::execute_di ()
  {
    set_flag(processor_flag_type::interrupt_disable, true);
  }

  void processor::execute_ei ()
  {
    set_flag(processor_flag_type::interrupt_enable, true);
  }

  void processor::execute_daa ()
  {
    bool          negative    = check_flag(processor_flag_type::negative),
                  half_carry  = check_flag(processor_flag_type::half_carry),
                  carry       = check_flag(processor_flag_type::carry),
                  new_carry   = false;
    std::uint8_t  adjustment  = 0;

    if (
      half_carry == true || 
      (negative == false && (m_registers[0] & 0xF) > 9)
    ) {
      adjustment = 0x06;
    }

    if (
      carry == true ||
      (negative == false && m_registers[0] > 0x99)
    ) {
      adjustment |= 0x60;
      new_carry = true;
    }

    m_registers[0] += (negative == true) ? -adjustment : adjustment;

    set_flag(processor_flag_type::zero, m_registers[0] == 0x00);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry, new_carry);
  }

  void processor::execute_cpl ()
  {
    m_registers[0] = ~m_registers[0];
    set_flag(processor_flag_type::negative, true);
    set_flag(processor_flag_type::half_carry, true);
  }

  void processor::execute_ccf ()
  {
    bool carry = check_flag(processor_flag_type::carry);

    set_flag(processor_flag_type::negative, false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry, !carry);
  }

  void processor::execute_scf ()
  {
    set_flag(processor_flag_type::negative, false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry, true);
  }

  /** 10XX. Data Transfer Instructions - Load Instructions ****************************************/

  void processor::execute_ld_i8 (memory& mem, const processor_register_type& dest_reg)
  {
    std::uint8_t value = mem.read_byte(m_program_counter); advance(1);
    write_register(dest_reg, value);
  }

  void processor::execute_ld_i16 (memory& mem, const processor_register_type& dest_reg)
  {
    std::uint16_t value = mem.read_word(m_program_counter); advance(2);
    write_register(dest_reg, value);
  }

  void processor::execute_ld_i32 (memory& mem, const processor_register_type& dest_reg)
  {
    std::uint32_t value = mem.read_long(m_program_counter); advance(4);
    write_register(dest_reg, value);
  }

  void processor::execute_ld_a32 (memory& mem, const processor_register_type& dest_reg)
  {
    std::uint32_t address = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  value   = mem.read_byte(address); cycle(1);
    write_register(dest_reg, value);
  }

  void processor::execute_ld_r32 (memory& mem, const processor_register_type& dest_reg,
    const processor_register_type& src_reg)
  {
    std::uint32_t address = read_register(src_reg);
    std::uint8_t  value   = mem.read_byte(address); cycle(1);
    write_register(dest_reg, value);
  }

  void processor::execute_lhb (memory& mem)
  {
    std::uint8_t  address_low_byte  = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  value             = mem.read_byte(0xFFFFFF00 + address_low_byte); cycle(1);
    write_register(processor_register_type::b0, value);
  }

  void processor::execute_lhr (memory& mem)
  {
    std::uint8_t  address_low_byte  = read_register(processor_register_type::b1);
    std::uint8_t  value             = mem.read_byte(0xFFFFFF00 + address_low_byte); cycle(1);
    write_register(processor_register_type::b0, value);
  }

  void processor::execute_lhw (memory& mem)
  {
    std::uint8_t  address_low_word  = mem.read_word(m_program_counter); advance(2);
    std::uint8_t  value             = mem.read_byte(0xFFFE0000 + address_low_word); cycle(1);
    write_register(processor_register_type::b0, value);
  }

  /** 11XX. Data Transfer Instructions - Store Instructions ***************************************/

  void processor::execute_st_a32 (memory& mem, const processor_register_type& src_reg)
  {
    std::uint32_t address = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  value   = read_register(src_reg);
    mem.write_byte(address, value); cycle(1);
  }

  void processor::execute_st_r32 (memory& mem, const processor_register_type& src_reg,
    const processor_register_type& addr_reg)
  {
    std::uint32_t address = read_register(addr_reg);
    std::uint8_t  value   = read_register(src_reg);
    mem.write_byte(address, value); cycle(1);
  }

  void processor::execute_shb (memory& mem)
  {
    std::uint8_t  address_low_byte  = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  value             = read_register(processor_register_type::b0);
    mem.write_byte(0xFFFFFF00 + address_low_byte, value); cycle(1);
  }

  void processor::execute_shr (memory& mem)
  {
    std::uint8_t  address_low_byte  = read_register(processor_register_type::b1);
    std::uint8_t  value             = read_register(processor_register_type::b0);
    mem.write_byte(0xFFFFFF00 + address_low_byte, value); cycle(1);
  }

  void processor::execute_shw (memory& mem)
  {
    std::uint16_t address_low_word  = mem.read_word(m_program_counter); advance(2);
    std::uint8_t  value             = read_register(processor_register_type::b0);
    mem.write_byte(0xFFFE0000 + address_low_word, value); cycle(1);
  }

  void processor::execute_ssp (memory& mem)
  {
    std::uint32_t address = mem.read_long(m_program_counter); advance(4);
    mem.write_word(address, m_stack_pointer); cycle(2);
  }

  void processor::execute_spc (memory& mem)
  {
    std::uint32_t address = mem.read_long(m_program_counter); advance(4);
    mem.write_long(address, m_program_counter); cycle(4);
  }

  /** 12XX. Data Transfer Instructions - Move Instructions ****************************************/

  void processor::execute_mv (const processor_register_type& dest_reg, 
    const processor_register_type& src_reg)
  {
    write_register(dest_reg, read_register(src_reg));
  }

  void processor::execute_msp (const processor_register_type& dest_reg)
  {
    write_register(dest_reg, m_stack_pointer);
  }

  void processor::execute_mpc (const processor_register_type& dest_reg)
  {
    write_register(dest_reg, m_program_counter);
  }

  /** 16XX. Data Transfer Instructions - Stack Instructions */

  void processor::execute_push (memory& mem, const processor_register_type& src_reg)
  {
    mem.push_long(m_stack_pointer, read_register(src_reg)); cycle(4);
  }
  
  void processor::execute_pop (memory& mem, const processor_register_type& dest_reg)
  {
    std::uint32_t value = mem.pop_long(m_stack_pointer); cycle(4);
    write_register(dest_reg, value);
  }

  /** 20XX. Control Transfer Instructions - Jumps *************************************************/
  
  void processor::execute_jmp_a32 (memory& mem, const processor_condition_type& cond)
  {
    std::uint32_t address = mem.read_long(m_program_counter); advance(4);
    if (check_condition(cond) == true) {
      m_program_counter = address; cycle(1);
    }
  }

  void processor::execute_jmp_r32 (const processor_condition_type& cond,
    const processor_register_type& addr_reg)
  {
    std::uint32_t address = read_register(addr_reg);
    if (check_condition(cond) == true) {
      m_program_counter = address; cycle(1);
    }
  }

  /** 22XX. Control Transfer Instructions - Calls *************************************************/

  void processor::execute_call_a32 (memory& mem, const processor_condition_type& cond)
  {
    std::uint32_t address = mem.read_long(m_program_counter); advance(4);
    if (check_condition(cond) == true) {
      mem.push_long(m_stack_pointer, m_program_counter); cycle(4);
      m_program_counter = address; cycle(1);
    }
  }

  void processor::execute_rst (memory& mem)
  {
    std::uint8_t  rst_vector = mem.read_byte(m_program_counter); advance(1);
    if (rst_vector > 7) { rst_vector = 0; }

    std::uint32_t rst_addr   = 0x10 * (rst_vector & 0b111);

    mem.push_long(m_stack_pointer, m_program_counter); cycle(4);
    m_program_counter = rst_addr; cycle(1);
  }

  void processor::execute_rst0 ()
  {
    m_program_counter = 0x00;
  }

  /** 23XX. Control Transfer Instructions - Returns ***********************************************/

  void processor::execute_ret (memory& mem, const processor_condition_type& cond)
  {
    if (check_condition(cond) == true) {
      std::uint32_t address = mem.pop_long(m_stack_pointer); cycle(4);
      m_program_counter = address; cycle(1);
    }
  }

  void processor::execute_reti (memory& mem)
  {
    set_flag(processor_flag_type::interrupt_disable, false);
    execute_ret(mem, processor_condition_type::none);
  }

  /** 30XX. Arithmetic Instructions - Increments **************************************************/

  void processor::execute_inc_r8 (const processor_register_type& reg)
  {
    std::uint8_t old_value = read_register(reg);
    std::uint8_t new_value = old_value + 1;
    write_register(reg, new_value);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, (new_value & 0xF) == 0x00);
  }

  void processor::execute_inc_r16 (const processor_register_type& reg)
  {
    std::uint16_t old_value = read_register(reg);
    std::uint16_t new_value = old_value + 1;
    write_register(reg, new_value);

    set_flag(processor_flag_type::zero,       new_value == 0x0000);
    set_flag(processor_flag_type::negative,   false);
  }

  void processor::execute_inc_r32 (const processor_register_type& reg)
  {
    std::uint32_t old_value = read_register(reg);
    std::uint32_t new_value = old_value + 1;
    write_register(reg, new_value);

    set_flag(processor_flag_type::zero,       new_value == 0x00000000);
    set_flag(processor_flag_type::negative,   false);
  }

  void processor::execute_inc_a32 (memory& mem)
  {
    std::uint32_t address = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value = old_value + 1;
    mem.write_byte(address, new_value); cycle(1);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, (new_value & 0xF) == 0x00);
  }

  void processor::execute_inc_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address = read_register(addr_reg);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value = old_value + 1;
    mem.write_byte(address, new_value); cycle(1);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, (new_value & 0xF) == 0x00);
  }

  /** 31XX. Arithmetic Instructions - Decrements **************************************************/

  void processor::execute_dec_r8 (const processor_register_type& reg)
  {
    std::uint8_t old_value = read_register(reg);
    std::uint8_t new_value = old_value - 1;
    write_register(reg, new_value);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, (new_value & 0xF) == 0x0F);
  }

  void processor::execute_dec_r16 (const processor_register_type& reg)
  {
    std::uint16_t old_value = read_register(reg);
    std::uint16_t new_value = old_value - 1;
    write_register(reg, new_value);

    set_flag(processor_flag_type::zero,       new_value == 0x0000);
    set_flag(processor_flag_type::negative,   true);
  }

  void processor::execute_dec_r32 (const processor_register_type& reg)
  {
    std::uint32_t old_value = read_register(reg);
    std::uint32_t new_value = old_value - 1;
    write_register(reg, new_value);

    set_flag(processor_flag_type::zero,       new_value == 0x00000000);
    set_flag(processor_flag_type::negative,   true);
  }

  void processor::execute_dec_a32 (memory& mem)
  {
    std::uint32_t address = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value = old_value - 1;
    mem.write_byte(address, new_value); cycle(1);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, (new_value & 0xF) == 0x0F);
  }

  void processor::execute_dec_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address = read_register(addr_reg);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value = old_value - 1;
    mem.write_byte(address, new_value); cycle(1);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, (new_value & 0xF) == 0x0F);
  }

  /** 32XX. Arithmetic Instructions - Addition */

  void processor::execute_add_i8 (memory& mem)
  {
    std::uint8_t  amount_to_add   = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  old_value       = m_registers[0];
    std::uint16_t new_value       = old_value + amount_to_add;
    std::uint8_t  low_nibble      = (old_value & 0xF) + (amount_to_add & 0xF);

    m_registers[0] = (new_value & 0xFF);

    set_flag(processor_flag_type::zero,       (new_value & 0xFF) == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, low_nibble > 0xF);
    set_flag(processor_flag_type::carry,      new_value > 0xFF);
  }

  void processor::execute_add_r8 (const processor_register_type& reg)
  {
    std::uint8_t  amount_to_add   = read_register(reg);
    std::uint8_t  old_value       = m_registers[0];
    std::uint16_t new_value       = old_value + amount_to_add;
    std::uint8_t  low_nibble      = (old_value & 0xF) + (amount_to_add & 0xF);

    m_registers[0] = (new_value & 0xFF);

    set_flag(processor_flag_type::zero,       (new_value & 0xFF) == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, low_nibble > 0xF);
    set_flag(processor_flag_type::carry,      new_value > 0xFF);
  }

  void processor::execute_add_a32 (memory& mem)
  {
    std::uint32_t address         = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  amount_to_add   = mem.read_byte(address); cycle(1);
    std::uint8_t  old_value       = m_registers[0];
    std::uint16_t new_value       = old_value + amount_to_add;
    std::uint8_t  low_nibble      = (old_value & 0xF) + (amount_to_add & 0xF);

    m_registers[0] = (new_value & 0xFF);

    set_flag(processor_flag_type::zero,       (new_value & 0xFF) == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, low_nibble > 0xF);
    set_flag(processor_flag_type::carry,      new_value > 0xFF);
  }

  void processor::execute_add_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address         = read_register(addr_reg);
    std::uint8_t  amount_to_add   = mem.read_byte(address); cycle(1);
    std::uint8_t  old_value       = m_registers[0];
    std::uint16_t new_value       = old_value + amount_to_add;
    std::uint8_t  low_nibble      = (old_value & 0xF) + (amount_to_add & 0xF);

    m_registers[0] = (new_value & 0xFF);

    set_flag(processor_flag_type::zero,       (new_value & 0xFF) == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, low_nibble > 0xF);
    set_flag(processor_flag_type::carry,      new_value > 0xFF);
  }

  void processor::execute_adc_i8 (memory& mem)
  {
    bool          carry           = check_flag(processor_flag_type::carry);
    std::uint8_t  amount_to_add   = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  old_value       = m_registers[0];
    std::uint16_t new_value       = old_value + amount_to_add + carry;
    std::uint8_t  low_nibble      = (old_value & 0xF) + (amount_to_add & 0xF) + carry;

    m_registers[0] = (new_value & 0xFF);

    set_flag(processor_flag_type::zero,       (new_value & 0xFF) == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, low_nibble > 0xF);
    set_flag(processor_flag_type::carry,      new_value > 0xFF);
  }

  void processor::execute_adc_r8 (const processor_register_type& reg)
  {
    bool          carry           = check_flag(processor_flag_type::carry);
    std::uint8_t  amount_to_add   = read_register(reg);
    std::uint8_t  old_value       = m_registers[0];
    std::uint16_t new_value       = old_value + amount_to_add + carry;
    std::uint8_t  low_nibble      = (old_value & 0xF) + (amount_to_add & 0xF) + carry;

    m_registers[0] = (new_value & 0xFF);

    set_flag(processor_flag_type::zero,       (new_value & 0xFF) == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, low_nibble > 0xF);
    set_flag(processor_flag_type::carry,      new_value > 0xFF);
  }

  void processor::execute_adc_a32 (memory& mem)
  {
    bool          carry           = check_flag(processor_flag_type::carry);
    std::uint32_t address         = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  amount_to_add   = mem.read_byte(address); cycle(1);
    std::uint8_t  old_value       = m_registers[0];
    std::uint16_t new_value       = old_value + amount_to_add + carry;
    std::uint8_t  low_nibble      = (old_value & 0xF) + (amount_to_add & 0xF) + carry;

    m_registers[0] = (new_value & 0xFF);

    set_flag(processor_flag_type::zero,       (new_value & 0xFF) == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, low_nibble > 0xF);
    set_flag(processor_flag_type::carry,      new_value > 0xFF);
  }

  void processor::execute_adc_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    bool          carry           = check_flag(processor_flag_type::carry);
    std::uint32_t address         = read_register(addr_reg);
    std::uint8_t  amount_to_add   = mem.read_byte(address); cycle(1);
    std::uint8_t  old_value       = m_registers[0];
    std::uint16_t new_value       = old_value + amount_to_add + carry;
    std::uint8_t  low_nibble      = (old_value & 0xF) + (amount_to_add & 0xF) + carry;

    m_registers[0] = (new_value & 0xFF);

    set_flag(processor_flag_type::zero,       (new_value & 0xFF) == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, low_nibble > 0xF);
    set_flag(processor_flag_type::carry,      new_value > 0xFF);
  }

  /** 33XX. Arithmetic Instructions - Subtraction *************************************************/
  
  void processor::execute_sub_i8 (memory& mem)
  {
    std::uint8_t  amount_to_subtract  = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  old_value           = m_registers[0];
    std::int16_t  new_value           = old_value - amount_to_subtract;
    std::int8_t   low_nibble          = (old_value & 0xF) - (amount_to_subtract & 0xF);

    m_registers[0] = (static_cast<std::uint16_t>(new_value) & 0xFF);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      new_value < 0);
  }

  void processor::execute_sub_r8 (const processor_register_type& reg)
  {
    std::uint8_t  amount_to_subtract  = read_register(reg);
    std::uint8_t  old_value           = m_registers[0];
    std::int16_t  new_value           = old_value - amount_to_subtract;
    std::int8_t   low_nibble          = (old_value & 0xF) - (amount_to_subtract & 0xF);

    m_registers[0] = (static_cast<std::uint16_t>(new_value) & 0xFF);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      new_value < 0);
  }

  void processor::execute_sub_a32 (memory& mem)
  {
    std::uint32_t address             = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  amount_to_subtract  = mem.read_byte(address); cycle(1);
    std::uint8_t  old_value           = m_registers[0];
    std::int16_t  new_value           = old_value - amount_to_subtract;
    std::int8_t   low_nibble          = (old_value & 0xF) - (amount_to_subtract & 0xF);

    m_registers[0] = (static_cast<std::uint16_t>(new_value) & 0xFF);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      new_value < 0);
  }

  void processor::execute_sub_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address             = read_register(addr_reg);
    std::uint8_t  amount_to_subtract  = mem.read_byte(address); cycle(1);
    std::uint8_t  old_value           = m_registers[0];
    std::int16_t  new_value           = old_value - amount_to_subtract;
    std::int8_t   low_nibble          = (old_value & 0xF) - (amount_to_subtract & 0xF);

    m_registers[0] = (static_cast<std::uint16_t>(new_value) & 0xFF);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      new_value < 0);
  }

  void processor::execute_sbc_i8 (memory& mem)
  {
    bool          carry               = check_flag(processor_flag_type::carry);
    std::uint8_t  amount_to_subtract  = mem.read_byte(m_program_counter) + carry; advance(1);
    std::uint8_t  old_value           = m_registers[0];
    std::int16_t  new_value           = old_value - amount_to_subtract;
    std::int8_t   low_nibble          = (old_value & 0xF) - (amount_to_subtract & 0xF);

    m_registers[0] = (static_cast<std::uint16_t>(new_value) & 0xFF);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      new_value < 0);
  }

  void processor::execute_sbc_r8 (const processor_register_type& reg)
  {
    bool          carry               = check_flag(processor_flag_type::carry);
    std::uint8_t  amount_to_subtract  = read_register(reg) + carry;
    std::uint8_t  old_value           = m_registers[0];
    std::int16_t  new_value           = old_value - amount_to_subtract;
    std::int8_t   low_nibble          = (old_value & 0xF) - (amount_to_subtract & 0xF);

    m_registers[0] = (static_cast<std::uint16_t>(new_value) & 0xFF);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      new_value < 0);
  }

  void processor::execute_sbc_a32 (memory& mem)
  {
    bool          carry               = check_flag(processor_flag_type::carry);
    std::uint32_t address             = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  amount_to_subtract  = mem.read_byte(address) + carry; cycle(1);
    std::uint8_t  old_value           = m_registers[0];
    std::int16_t  new_value           = old_value - amount_to_subtract;
    std::int8_t   low_nibble          = (old_value & 0xF) - (amount_to_subtract & 0xF);

    m_registers[0] = (static_cast<std::uint16_t>(new_value) & 0xFF);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      new_value < 0);
  }

  void processor::execute_sbc_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    bool          carry               = check_flag(processor_flag_type::carry);
    std::uint32_t address             = read_register(addr_reg);
    std::uint8_t  amount_to_subtract  = mem.read_byte(address) + carry; cycle(1);
    std::uint8_t  old_value           = m_registers[0];
    std::int16_t  new_value           = old_value - amount_to_subtract;
    std::int8_t   low_nibble          = (old_value & 0xF) - (amount_to_subtract & 0xF);

    m_registers[0] = (static_cast<std::uint16_t>(new_value) & 0xFF);

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      new_value < 0);
  }

  /** 50XX. Logical Instructions - AND ************************************************************/

  void processor::execute_and_i8 (memory& mem)
  {
    std::uint8_t righthand = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t new_value = (m_registers[0] & righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, true);
    set_flag(processor_flag_type::carry,      false);
  }

  void processor::execute_and_r8 (const processor_register_type& reg)
  {
    std::uint8_t righthand = read_register(reg);
    std::uint8_t new_value = (m_registers[0] & righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, true);
    set_flag(processor_flag_type::carry,      false);
  }

  void processor::execute_and_a32 (memory& mem)
  {
    std::uint32_t address   = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  righthand = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value = (m_registers[0] & righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, true);
    set_flag(processor_flag_type::carry,      false);
  }

  void processor::execute_and_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address   = read_register(addr_reg);
    std::uint8_t  righthand = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value = (m_registers[0] & righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, true);
    set_flag(processor_flag_type::carry,      false);
  }

  /** 51XX. Logical Instructions - OR ************************************************************/

  void processor::execute_or_i8 (memory& mem)
  {
    std::uint8_t righthand = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t new_value = (m_registers[0] | righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      false);
  }

  void processor::execute_or_r8 (const processor_register_type& reg)
  {
    std::uint8_t righthand = read_register(reg);
    std::uint8_t new_value = (m_registers[0] | righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      false);
  }

  void processor::execute_or_a32 (memory& mem)
  {
    std::uint32_t address   = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  righthand = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value = (m_registers[0] | righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      false);
  }

  void processor::execute_or_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address   = read_register(addr_reg);
    std::uint8_t  righthand = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value = (m_registers[0] | righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      false);
  }

  /** 52XX. Logical Instructions - XOR ************************************************************/

  void processor::execute_xor_i8 (memory& mem)
  {
    std::uint8_t righthand = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t new_value = (m_registers[0] ^ righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      false);
  }

  void processor::execute_xor_r8 (const processor_register_type& reg)
  {
    std::uint8_t righthand = read_register(reg);
    std::uint8_t new_value = (m_registers[0] ^ righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      false);
  }

  void processor::execute_xor_a32 (memory& mem)
  {
    std::uint32_t address   = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  righthand = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value = (m_registers[0] ^ righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      false);
  }

  void processor::execute_xor_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address   = read_register(addr_reg);
    std::uint8_t  righthand = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value = (m_registers[0] ^ righthand);

    m_registers[0] = new_value;

    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      false);
  }

  /** 53XX. Logical Instructions - CMP ************************************************************/

  void processor::execute_cmp_i8 (memory& mem)
  {
    std::uint8_t  amount_to_subtract  = mem.read_byte(m_program_counter); advance(1);
    std::int16_t  difference          = m_registers[0] - amount_to_subtract;
    std::int8_t   low_nibble          = (m_registers[0] & 0xF) - (amount_to_subtract & 0xF);

    set_flag(processor_flag_type::zero,       difference == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      difference < 0);
  }

  void processor::execute_cmp_r8 (const processor_register_type& reg)
  {
    std::uint8_t  amount_to_subtract  = read_register(reg);
    std::int16_t  difference          = m_registers[0] - amount_to_subtract;
    std::int8_t   low_nibble          = (m_registers[0] & 0xF) - (amount_to_subtract & 0xF);

    set_flag(processor_flag_type::zero,       difference == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      difference < 0);
  }

  void processor::execute_cmp_a32 (memory& mem)
  {
    std::uint32_t address             = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  amount_to_subtract  = mem.read_byte(address); cycle(1);
    std::int16_t  difference          = m_registers[0] - amount_to_subtract;
    std::int8_t   low_nibble          = (m_registers[0] & 0xF) - (amount_to_subtract & 0xF);

    set_flag(processor_flag_type::zero,       difference == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      difference < 0);
  }

  void processor::execute_cmp_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address             = read_register(addr_reg);
    std::uint8_t  amount_to_subtract  = mem.read_byte(address); cycle(1);
    std::int16_t  difference          = m_registers[0] - amount_to_subtract;
    std::int8_t   low_nibble          = (m_registers[0] & 0xF) - (amount_to_subtract & 0xF);

    set_flag(processor_flag_type::zero,       difference == 0x00);
    set_flag(processor_flag_type::negative,   true);
    set_flag(processor_flag_type::half_carry, low_nibble < 0);
    set_flag(processor_flag_type::carry,      difference < 0);
  }

  /** 60XX. Bitwise Instructions - BIT ************************************************************/

  void processor::execute_bit_r8 (memory& mem, const processor_register_type& src_reg)
  {
    std::uint8_t  bit_number  = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  value       = read_register(src_reg);

    set_flag(processor_flag_type::zero,       sm_getbit(value, (bit_number & 0b111)) == 0);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, true);
  }

  void processor::execute_bit_a32 (memory& mem)
  {
    std::uint8_t  bit_number  = mem.read_byte(m_program_counter); advance(1);
    std::uint32_t address     = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  value       = mem.read_byte(address);           cycle(1);

    set_flag(processor_flag_type::zero,       sm_getbit(value, (bit_number & 0b111)) == 0);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, true);
  }

  void processor::execute_bit_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address     = read_register(addr_reg);
    std::uint8_t  bit_number  = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  value       = mem.read_long(address); cycle(4);

    set_flag(processor_flag_type::zero,       sm_getbit(value, (bit_number & 0b111)) == 0);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, true);
  }

  /** 61XX. Bitwise Instructions - SET ************************************************************/

  void processor::execute_set_r8 (memory& mem, const processor_register_type& src_reg)
  {
    std::uint8_t  bit_number  = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  value       = read_register(src_reg);

    sm_setbit(value, (bit_number & 0b111), true);
    write_register(src_reg, value);
  }

  void processor::execute_set_a32 (memory& mem)
  {
    std::uint8_t  bit_number  = mem.read_byte(m_program_counter); advance(1);
    std::uint32_t address     = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  value       = mem.read_byte(address);           cycle(1);

    sm_setbit(value, (bit_number & 0b111), true);
    mem.write_byte(address, value); cycle(1);
  }

  void processor::execute_set_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address     = read_register(addr_reg);
    std::uint8_t  bit_number  = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  value       = mem.read_long(address); cycle(4);

    sm_setbit(value, (bit_number & 0b111), true);
    mem.write_byte(address, value); cycle(1);
  }

  /** 62XX. Bitwise Instructions - RES ************************************************************/

  void processor::execute_res_r8 (memory& mem, const processor_register_type& src_reg)
  {
    std::uint8_t  bit_number  = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  value       = read_register(src_reg);

    sm_setbit(value, (bit_number & 0b111), false);
    write_register(src_reg, value);
  }

  void processor::execute_res_a32 (memory& mem)
  {
    std::uint8_t  bit_number  = mem.read_byte(m_program_counter); advance(1);
    std::uint32_t address     = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  value       = mem.read_byte(address);           cycle(1);

    sm_setbit(value, (bit_number & 0b111), false);
    mem.write_byte(address, value); cycle(1);
  }

  void processor::execute_res_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address     = read_register(addr_reg);
    std::uint8_t  bit_number  = mem.read_byte(m_program_counter); advance(1);
    std::uint8_t  value       = mem.read_long(address); cycle(4);

    sm_setbit(value, (bit_number & 0b111), false);
    mem.write_byte(address, value); cycle(1);
  }

  /** 70XX. Shift and Rotate Instructions - SLA ***************************************************/

  void processor::execute_sla_r8 (const processor_register_type& reg)
  {
    std::uint8_t  old_value = read_register(reg);
    std::uint8_t  new_value = (old_value << 1) | 0;
    bool          new_carry = sm_getbit(old_value, 7);

    write_register(reg, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_sla_a32 (memory& mem)
  {
    std::uint32_t address     = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  old_value   = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value   = (old_value << 1) | 0;
    bool          new_carry   = sm_getbit(old_value, 7);

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_sla_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address     = read_register(addr_reg);
    std::uint8_t  old_value   = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value   = (old_value << 1) | 0;
    bool          new_carry   = sm_getbit(old_value, 7);

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  /** 71XX. Shift and Rotate Instructions - SRA ***************************************************/

  void processor::execute_sra_r8 (const processor_register_type& reg)
  {
    std::uint8_t  old_value   = read_register(reg);
    std::uint8_t  new_value   = (old_value >> 1) | (old_value & 0b10000000);
    bool          new_carry   = sm_getbit(old_value, 0);

    write_register(reg, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_sra_a32 (memory& mem)
  {
    std::uint32_t address     = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  old_value   = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value   = (old_value >> 1) | (old_value & 0b10000000);
    bool          new_carry   = sm_getbit(old_value, 0);

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_sra_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address     = read_register(addr_reg);
    std::uint8_t  old_value   = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value   = (old_value >> 1) | (old_value & 0b10000000);
    bool          new_carry   = sm_getbit(old_value, 0);

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  /** 72XX. Shift and Rotate Instructions - SRL ***************************************************/

  void processor::execute_srl_r8 (const processor_register_type& reg)
  {
    std::uint8_t  old_value   = read_register(reg);
    std::uint8_t  new_value   = (old_value >> 1);
    bool          new_carry   = sm_getbit(old_value, 0);

    write_register(reg, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_srl_a32 (memory& mem)
  {
    std::uint32_t address     = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  old_value   = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value   = (old_value >> 1);
    bool          new_carry   = sm_getbit(old_value, 0);

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_srl_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address     = read_register(addr_reg);
    std::uint8_t  old_value   = mem.read_byte(address); cycle(1);
    std::uint8_t  new_value   = (old_value >> 1);
    bool          new_carry   = sm_getbit(old_value, 0);

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  /** 73XX. Shift and Rotate Instructions - RL ****************************************************/

  void processor::execute_rl_r8 (const processor_register_type& reg)
  {
    bool          old_carry = check_flag(processor_flag_type::carry);
    std::uint8_t  old_value = read_register(reg);
    bool          new_carry = sm_getbit(old_value, 7);
    std::uint8_t  new_value = (old_value << 1) | old_carry;

    write_register(reg, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rl_a32 (memory& mem)
  {
    std::uint32_t address   = mem.read_long(m_program_counter); advance(4);
    bool          old_carry = check_flag(processor_flag_type::carry);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    bool          new_carry = sm_getbit(old_value, 7);
    std::uint8_t  new_value = (old_value << 1) | old_carry;

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rl_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address   = read_register(addr_reg);
    bool          old_carry = check_flag(processor_flag_type::carry);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    bool          new_carry = sm_getbit(old_value, 7);
    std::uint8_t  new_value = (old_value << 1) | old_carry;

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rla ()
  {
    bool          old_carry = check_flag(processor_flag_type::carry);
    std::uint8_t  old_value = read_register(processor_register_type::b0);
    bool          new_carry = sm_getbit(old_value, 7);
    std::uint8_t  new_value = (old_value << 1) | old_carry;

    write_register(processor_register_type::b0, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  /** 74XX. Shift and Rotate Instructions - RLC ***************************************************/

  void processor::execute_rlc_r8 (const processor_register_type& reg)
  {
    std::uint8_t  old_value = read_register(reg);
    bool          new_carry = sm_getbit(old_value, 7);
    std::uint8_t  new_value = (old_value << 1) | new_carry;

    write_register(reg, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rlc_a32 (memory& mem)
  {
    std::uint32_t address   = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    bool          new_carry = sm_getbit(old_value, 7);
    std::uint8_t  new_value = (old_value << 1) | new_carry;

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rlc_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address   = read_register(addr_reg);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    bool          new_carry = sm_getbit(old_value, 7);
    std::uint8_t  new_value = (old_value << 1) | new_carry;

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rlca ()
  {
    std::uint8_t  old_value = read_register(processor_register_type::b0);
    bool          new_carry = sm_getbit(old_value, 7);
    std::uint8_t  new_value = (old_value << 1) | new_carry;

    write_register(processor_register_type::b0, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  /** 73XX. Shift and Rotate Instructions - RR ****************************************************/

  void processor::execute_rr_r8 (const processor_register_type& reg)
  {
    std::uint8_t  old_carry = check_flag(processor_flag_type::carry);
    std::uint8_t  old_value = read_register(reg);
    bool          new_carry = sm_getbit(old_value, 0);
    std::uint8_t  new_value = (old_value >> 1) | (old_carry << 7);

    write_register(reg, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rr_a32 (memory& mem)
  {
    std::uint32_t address   = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  old_carry = check_flag(processor_flag_type::carry);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    bool          new_carry = sm_getbit(old_value, 0);
    std::uint8_t  new_value = (old_value >> 1) | (old_carry << 7);

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rr_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address   = read_register(addr_reg);
    std::uint8_t  old_carry = check_flag(processor_flag_type::carry);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    bool          new_carry = sm_getbit(old_value, 0);
    std::uint8_t  new_value = (old_value >> 1) | (old_carry << 7);

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rra ()
  {
    std::uint8_t  old_carry = check_flag(processor_flag_type::carry);
    std::uint8_t  old_value = read_register(processor_register_type::b0);
    bool          new_carry = sm_getbit(old_value, 0);
    std::uint8_t  new_value = (old_value >> 1) | (old_carry << 7);

    write_register(processor_register_type::b0, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  /** 74XX. Shift and Rotate Instructions - RRC ***************************************************/

  void processor::execute_rrc_r8 (const processor_register_type& reg)
  {
    std::uint8_t  old_value = read_register(reg);
    bool          new_carry = sm_getbit(old_value, 0);
    std::uint8_t  new_value = (old_value >> 1) | (old_value << 7);

    write_register(reg, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rrc_a32 (memory& mem)
  {
    std::uint32_t address   = mem.read_long(m_program_counter); advance(4);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    bool          new_carry = sm_getbit(old_value, 0);
    std::uint8_t  new_value = (old_value >> 1) | (old_value << 7);

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rrc_ar32 (memory& mem, const processor_register_type& addr_reg)
  {
    std::uint32_t address   = read_register(addr_reg);
    std::uint8_t  old_value = mem.read_byte(address); cycle(1);
    bool          new_carry = sm_getbit(old_value, 0);
    std::uint8_t  new_value = (old_value >> 1) | (old_value << 7);

    mem.write_byte(address, new_value); cycle(1);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

  void processor::execute_rrca ()
  {
    std::uint8_t  old_value = read_register(processor_register_type::b0);
    bool          new_carry = sm_getbit(old_value, 0);
    std::uint8_t  new_value = (old_value >> 1) | (old_value << 7);

    write_register(processor_register_type::b0, new_value);
    set_flag(processor_flag_type::zero,       new_value == 0x00);
    set_flag(processor_flag_type::negative,   false);
    set_flag(processor_flag_type::half_carry, false);
    set_flag(processor_flag_type::carry,      new_carry); 
  }

}
