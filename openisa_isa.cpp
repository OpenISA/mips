// @file      openisa_isa.cpp
// @brief     The ArchC OpenISA functional model.

#include "openisa_isa.H"
#include "openisa_bhv_macros.H"
#include "openisa_isa_init.cpp"
#include <cfenv>

// If you want debug information for this model, uncomment next line
//#define DEBUG_MODEL
#include "ac_debug_model.H"

//! User defined macros to reference registers.
#define Ra 31
#define Sp 29

// 'using namespace' statement to allow access to all
// openisa-specific datatypes
using namespace openisa_parms;

static int processors_started = 0;
#define DEFAULT_STACK_SIZE (256 * 1024)

// Generic instruction behavior method.
void ac_behavior(instruction) {
  dbg_printf("----- PC=%#x ----- %lld\n", (int)ac_pc, ac_instr_counter);
  ac_pc += 4;
};

//  Instruction Format behavior methods.
void ac_behavior(PL26j) {}
void ac_behavior(PL26c) {}
void ac_behavior(PL26i) {}
void ac_behavior(PL26ij) {}
void ac_behavior(PL24) {}
void ac_behavior(PL20) {}
void ac_behavior(PL20i) {}
void ac_behavior(PL18i) {}
void ac_behavior(PL18) {}
void ac_behavior(PL16) {}
void ac_behavior(PL12) {}
void ac_behavior(PL6) {}
void ac_behavior(PL0) {}

// Behavior called before starting simulation
void ac_behavior(begin) {
  dbg_printf("@@@ begin behavior @@@\n");

  for (int regNum = 0; regNum < 64; regNum++)
    RB[regNum] = 0;
}

// Behavior called after finishing simulation
void ac_behavior(end) { dbg_printf("@@@ end behavior @@@\n"); }

// Individual instructions emulation code
void ac_behavior(ldb) {
  char byte;
  dbg_printf("ldb r%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* r%d <= MEM[%x]\n", rt, RB[rs] + imm);
  byte = DATA_PORT->read_byte(RB[rs] + imm);
  RB[rt] = (ac_Sword)byte;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(ldbu) {
  unsigned char byte;
  dbg_printf("ldbu r%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* r%d <= MEM[%x]\n", rt, RB[rs] + imm);
  byte = DATA_PORT->read_byte(RB[rs] + imm);
  RB[rt] = byte;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(ldh) {
  short int half;
  dbg_printf("ldh r%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* r%d <= MEM[%x]\n", rt, RB[rs] + imm);
  half = DATA_PORT->read_half(RB[rs] + imm);
  RB[rt] = (ac_Sword)half;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(ldhu) {
  unsigned short int half;
  dbg_printf("ldhu r%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* r%d <= MEM[%x]\n", rt, RB[rs] + imm);
  half = DATA_PORT->read_half(RB[rs] + imm);
  RB[rt] = half;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(ldw) {
  dbg_printf("ldw r%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* r%d <= MEM[%x]\n", rt, RB[rs] + imm);
  RB[rt] = DATA_PORT->read(RB[rs] + imm);
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(ldc1) {
  dbg_printf("ldc1 %%f%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* f%d <= MEM[%x]\n", rt, RB[rs] + imm);
  dbg_printf("* f%d <= MEM[%x]\n", rt + 1, RB[rs] + imm + 4);
  RBD[rt + 1] = DATA_PORT->read(RB[rs] + imm);
  RBD[rt] = DATA_PORT->read(RB[rs] + imm + 4);
  double temp = load_double(rt);
  dbg_printf("Result = %lf (%08x%08x)\n", temp, RBD[rt], RBD[rt + 1]);
}

void ac_behavior(lwc1) {
  dbg_printf("lwc1 %%f%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* f%d <= MEM[%x]\n", rt, RB[rs] + imm);
  RBS[rt] = DATA_PORT->read(RB[rs] + imm);
  float temp = load_float(rt);
  dbg_printf("Result = %f\n", temp);
}

void ac_behavior(ldxc1) {
  dbg_printf("ldxc1 %%f%d, %%%d(%%%d)\n", rd, rt, rs);
  dbg_printf("* f%d <= MEM[%x]\n", rd, RB[rs] + RB[rt]);
  dbg_printf("* f%d <= MEM[%x]\n", rd + 1, RB[rs] + RB[rt] + 4);
  RBD[rd + 1] = DATA_PORT->read(RB[rt] + RB[rs]);
  RBD[rd] = DATA_PORT->read(RB[rt] + RB[rs] + 4);
  double temp = load_double(rd);
  dbg_printf("Result = %lf\n", temp);
}

void ac_behavior(lwxc1) {
  dbg_printf("lwxc1 %%f%d, %%%d(%%%d)\n", rd, rt, rs);
  dbg_printf("* f%d <= MEM[%x]\n", rd, RB[rs] + RB[rt]);
  RBS[rd] = DATA_PORT->read(RB[rt] + RB[rs]);
  float temp = load_float(rd);
  dbg_printf("Result = %f\n", temp);
}

void ac_behavior(ldwl) {
  dbg_printf("ldwl r%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* r%d <= MEM[%x]\n", rt, RB[rs] + imm);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (addr & 0x3) * 8;
  data = DATA_PORT->read(addr & 0xFFFFFFFC);
  data <<= offset;
  data |= RB[rt] & ((1 << offset) - 1);
  RB[rt] = data;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(ldwr) {
  dbg_printf("ldwr r%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* r%d <= MEM[%x]\n", rt, RB[rs] + imm);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (3 - (addr & 0x3)) * 8;
  data = DATA_PORT->read(addr & 0xFFFFFFFC);
  data >>= offset;
  data |= RB[rt] & (0xFFFFFFFF << (32 - offset));
  RB[rt] = data;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(stb) {
  unsigned char byte;
  dbg_printf("stb r%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + imm, rt);
  byte = RB[rt] & 0xFF;
  DATA_PORT->write_byte(RB[rs] + imm, byte);
  dbg_printf("Result = %#x\n", (int)byte);
}

void ac_behavior(sth) {
  unsigned short int half;
  dbg_printf("sth r%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + imm, rt);
  half = RB[rt] & 0xFFFF;
  DATA_PORT->write_half(RB[rs] + imm, half);
  dbg_printf("Result = %#x\n", (int)half);
}

void ac_behavior(stw) {
  dbg_printf("stw r%d, %d(r%d)\n", rt, imm, rs);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + imm, rt);
  DATA_PORT->write(RB[rs] + imm, RB[rt]);
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(absd) {
  dbg_printf("abs.d %%f%d, %%f%d\n", rs, rt);
  double res = fabs(load_double(rt));
  save_double(res, rs);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior(abss) {
  dbg_printf("abs.s %%f%d, %%f%d\n", rs, rt);
  float res = fabsf(load_float(rt));
  save_float(res, rs);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior(addd) {
  dbg_printf("add.d %%f%d, %%f%d, %%f%d\n", rd, rs, rt);
  double res = load_double(rs) + load_double(rt);
  save_double(res, rd);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior(adds) {
  dbg_printf("add.s %%f%d, %%f%d, %%f%d\n", rd, rs, rt);
  float res = load_float(rs) + load_float(rt);
  save_float(res, rd);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior(ceqd) {
  dbg_printf("c.eq.d %%f%d, %%f%d\n", rs, rt);
  double a = load_double(rs);
  double b = load_double(rt);
  cc = a == b ? (custom_isnan(a) || custom_isnan(b) ? 0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(ceqs) {
  dbg_printf("c.eq.s %%f%d, %%f%d\n", rs, rt);
  float a = load_float(rs);
  float b = load_float(rt);
  cc = a == b ? (custom_isnanf(a) || custom_isnanf(b) ? 0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(coled) {
  dbg_printf("c.ole.d %%f%d, %%f%d\n", rs, rt);
  double a = load_double(rs);
  double b = load_double(rt);
  cc = a <= b ? (custom_isnan(a) || custom_isnan(b) ? 0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(coles) {
  dbg_printf("c.ole.s %%f%d, %%f%d\n", rs, rt);
  float a = load_float(rs);
  float b = load_float(rt);
  cc = a <= b ? (custom_isnanf(a) || custom_isnanf(b) ? 0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(coltd) {
  dbg_printf("c.olt.d %%f%d, %%f%d\n", rs, rt);
  double a = load_double(rs);
  double b = load_double(rt);
  cc = a < b ? (custom_isnan(a) || custom_isnan(b) ? 0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(colts) {
  dbg_printf("c.olt.s %%f%d, %%f%d\n", rs, rt);
  float a = load_float(rs);
  float b = load_float(rt);
  cc = a < b ? (custom_isnanf(a) || custom_isnanf(b) ? 0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(cueqd) {
  dbg_printf("c.ueq.d %%f%d, %%f%d\n", rs, rt);
  cc = (load_double(rs) == load_double(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(cueqs) {
  dbg_printf("c.ueq.s %%f%d, %%f%d\n", rs, rt);
  cc = (load_float(rs) == load_float(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(culed) {
  dbg_printf("c.ule.d %%f%d, %%f%d\n", rs, rt);
  cc = (load_double(rs) <= load_double(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(cules) {
  dbg_printf("c.ule.s %%f%d, %%f%d\n", rs, rt);
  cc = (load_float(rs) <= load_float(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(cultd) {
  dbg_printf("c.ult.d %%f%d, %%f%d\n", rs, rt);
  cc = (load_double(rs) < load_double(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(cults) {
  dbg_printf("c.ult.s %%f%d, %%f%d\n", rs, rt);
  cc = (load_float(rs) < load_float(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(cund) {
  dbg_printf("c.un.d %%f%d, %%f%d\n", rs, rt);
  cc = (custom_isnan(load_double(rs)) || custom_isnan(load_double(rt))) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(cuns) {
  dbg_printf("c.un.s %%f%d, %%f%d\n", rs, rt);
  cc = (custom_isnanf(load_float(rs)) || custom_isnanf(load_float(rt))) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior(cvtsd) {
  dbg_printf("cvt.s.d %%f%d, %%f%d\n", rs, rt);
  float temp = (float)load_double(rt);
  save_float(temp, rs);
  dbg_printf("Result = %f\n", temp);
}

void ac_behavior(cvtds) {
  dbg_printf("cvt.d.s %%f%d, %%f%d\n", rs, rt);
  double temp = (double)load_float(rt);
  save_double(temp, rs);
  dbg_printf("Result = %lf\n", temp);
}

void ac_behavior(cvtdw) {
  dbg_printf("cvt.d.w %%f%d, %%f%d\n", rs, rt);
  double temp = (double)(int)RBS[rt];
  save_double(temp, rs);
  dbg_printf("Result = %lf\n", temp);
}

void ac_behavior(cvtsw) {
  dbg_printf("cvt.s.w %%f%d, %%f%d\n", rs, rt);
  float temp = (float)(int)RBS[rt];
  save_float(temp, rs);
  dbg_printf("Result = %f\n", temp);
}

void ac_behavior(divd) {
  dbg_printf("div.d %%f%d, %%f%d, %%f%d\n", rd, rs, rt);
  double res = load_double(rs) / load_double(rt);
  save_double(res, rd);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior(divs) {
  dbg_printf("div.s %%f%d, %%f%d, %%f%d\n", rd, rs, rt);
  float res = load_float(rs) / load_float(rt);
  save_float(res, rd);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior(mfc1) {
  dbg_printf("mfc1 %%%d, %%f%d\n", rs, rt);
  RB[rs] = RBS[rt];
  dbg_printf("Result = 0x%X\n", RB[rs]);
}

void ac_behavior(movd) {
  dbg_printf("mov.d %%f%d, %%f%d\n", rs, rt);
  double res = load_double(rt);
  save_double(res, rs);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior(movs) {
  dbg_printf("mov.s %%f%d, %%f%d\n", rs, rt);
  float res = load_float(rt);
  save_float(res, rs);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior(muld) {
  dbg_printf("mul.d %%f%d, %%f%d, %%f%d\n", rd, rs, rt);
  double res = load_double(rs) * load_double(rt);
  save_double(res, rd);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior(muls) {
  dbg_printf("mul.s %%f%d, %%f%d, %%f%d\n", rd, rs, rt);
  float res = load_float(rs) * load_float(rt);
  save_float(res, rd);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior(mtc1) {
  dbg_printf("mtc1 %%%d, %%f%d\n", rs, rt);
  RBS[rt] = RB[rs];
}

void ac_behavior(negd) {
  dbg_printf("neg.d %%f%d, %%f%d\n", rs, rt);
  double res = -load_double(rt);
  save_double(res, rs);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior(negs) {
  dbg_printf("neg.s %%f%d, %%f%d\n", rs, rt);
  float res = -load_float(rt);
  save_float(res, rs);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior(subd) {
  dbg_printf("sub.d %%f%d, %%f%d, %%f%d\n", rd, rs, rt);
  double res = load_double(rs) - load_double(rt);
  save_double(res, rd);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior(subs) {
  dbg_printf("sub.s %%f%d, %%f%d, %%f%d\n", rd, rs, rt);
  float res = load_float(rs) - load_float(rt);
  save_float(res, rd);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior(truncwd) {
  dbg_printf("trunc.w.d %%f%d, %%f%d\n", rs, rt);
  RBS[rs] = (int32_t)load_double(rt);
  dbg_printf("Result = %d\n", RBS[rs]);
}

void ac_behavior(truncws) {
  dbg_printf("trunc.w.s %%f%d, %%f%d\n", rs, rt);
  RBS[rs] = (int32_t)load_float(rt);
  dbg_printf("Result = %d\n", RBS[rs]);
}

void ac_behavior(sqrtd) {
  dbg_printf("sqrt.d %%f%d, %%f%d\n", rs, rt);
  double res = sqrt(load_double(rt));
  save_double(res, rs);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior(sqrts) {
  dbg_printf("sqrt.s %%f%d, %%f%d\n", rs, rt);
  float res = sqrtf(load_float(rt));
  save_float(res, rs);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior(bc1f) {
  dbg_printf("bc1f %d\n", halfword);
  if (cc == 0) {
    ac_pc = ac_pc + (halfword << 2);
    dbg_printf("Taken to %#x\n", ac_pc + (halfword << 2));
  }
}

void ac_behavior(bc1fl) {
  dbg_printf("bc1fl %d\n", halfword);
  if (cc == 0) {
    ac_pc = ac_pc + (halfword << 2);
    dbg_printf("Taken to %#x\n", ac_pc + (halfword << 2));
  }
}

void ac_behavior(bc1t) {
  dbg_printf("bc1t %d\n", halfword);
  if (cc == 1) {
    ac_pc = ac_pc + (halfword << 2);
    dbg_printf("Taken to %#x\n", ac_pc + (halfword << 2));
  }
}

void ac_behavior(bc1tl) {
  dbg_printf("bc1tl %d\n", halfword);
  if (cc == 1) {
    ac_pc = ac_pc + (halfword << 2);
    dbg_printf("Taken to %#x\n", ac_pc + (halfword << 2));
  }
}

void ac_behavior(rcall) {
  dbg_printf("rcall %d\n", halfword);
  RB[Ra] = ac_pc;
  ac_pc = ac_pc + (halfword << 2);

  //  dbg_printf("Return = %#x\n", RB[Ra].read());
  //  dbg_printf("Taken to %#x\n", ac_pc);

}

void ac_behavior(ijmp) {
  dbg_printf("ijmp %d(r%d), %d\n", pl12, index, count);
  ijmpreg &= 0xFFFFF000;
  ijmpreg |= pl12 & 0xFFF;
  uint32_t Target = DATA_PORT->read(ijmpreg + RB[index]);
  ac_pc = Target;
  dbg_printf("Jump table base = %#x\n", ijmpreg);
  dbg_printf("Target = %#x\n", Target);
}

void ac_behavior(ijmphi) {
  dbg_printf("ijmphi %d\n", pl20 & 0xFFFFF);
  ijmpreg = 0;
  ijmpreg |= pl20 << 12;
  dbg_printf("Result is: %#x\n", ijmpreg);
}

void ac_behavior(ldihi) {
  dbg_printf("ldihi %d\n", pl18 & 0x3FFFF);
  RB[ldireg] &= 0x3FFF;
  RB[ldireg] |= pl18 << 14;
  dbg_printf("Result is: %#x\n", RB[ldireg]);
}

void ac_behavior(sdc1) {
  dbg_printf("sdc1 %%f%d, %d(r%d)\n", rt, imm, rs);
  DATA_PORT->write(RB[rs] + imm + 4, RBD[rt]);
  DATA_PORT->write(RB[rs] + imm, RBD[rt + 1]);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + imm + 4, rt);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + imm, rt + 1);
}

void ac_behavior(swc1) {
  dbg_printf("swc1 %%f%d, %d(r%d)\n", rt, imm, rs);
  DATA_PORT->write(RB[rs] + imm, RBS[rt]);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + imm, rt);
}

void ac_behavior(sdxc1) {
  dbg_printf("sdxc1 %%f%d, %%%d(%%%d)\n", rd, rt, rs);
  DATA_PORT->write(RB[rt] + RB[rs] + 4, RBD[rd]);
  DATA_PORT->write(RB[rt] + RB[rs], RBD[rd + 1]);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + RB[rt] + 4, rd);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + RB[rt], rd + 1);
}

void ac_behavior(swxc1) {
  dbg_printf("swxc1 %%f%d, %%%d(%%%d)\n", rd, rt, rs);
  DATA_PORT->write(RB[rt] + RB[rs], RBS[rd]);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + RB[rt], rd);
}

void ac_behavior(mfhc1) {
  dbg_printf("mfhc1 %%%d, %%f%d\n", rs, rt);
  uint64_t temp;
  double input = load_double(rt);
  memcpy(&temp, &input, sizeof(uint64_t));
  RB[rs] = temp >> 32;
  dbg_printf("Result = 0x%X\n", RB[rs]);
}

void ac_behavior(mflc1) {
  dbg_printf("mflc1 %%%d, %%f%d\n", rs, rt);
  uint64_t temp;
  double input = load_double(rt);
  memcpy(&temp, &input, sizeof(uint64_t));
  RB[rs] = temp & 0xFFFFFFFF;
  dbg_printf("Result = 0x%X\n", RB[rs]);
}

void ac_behavior(mthc1) {
  dbg_printf("mthc1 %%%d, %%f%d\n", rs, rt);
  double temp = load_double(rt);
  uint64_t to_int;
  memcpy(&to_int, &temp, sizeof(uint64_t));
  to_int = (to_int & 0xFFFFFFFFULL) + (((uint64_t)RB[rs]) << 32);
  memcpy(&temp, &to_int, sizeof(uint64_t));
  save_double(temp, rt);
}

void ac_behavior(mtlc1) {
  dbg_printf("mtlc1 %%%d, %%f%d\n", rs, rt);
  double temp = load_double(rt);
  uint64_t to_int;
  memcpy(&to_int, &temp, sizeof(uint64_t));
  to_int = (to_int & 0xFFFFFFFF00000000ULL) + (((uint64_t)RB[rs]));
  memcpy(&temp, &to_int, sizeof(uint64_t));
  save_double(temp, rt);
}

void ac_behavior(roundwd) {
  dbg_printf("round.w.d %%%d, %%f%d\n", rs, rt);
  if (fesetround(FE_TONEAREST) == 0) {
    fprintf(stderr, "Failed to set rounding mode.\n");
    exit(EXIT_FAILURE);
  }
  int32_t res = (int) nearbyint(load_double(rt));
  RB[rt] = res;
  dbg_printf("Result = %d\n", res);
}

void ac_behavior(roundws) {
  dbg_printf("round.w.s %%%d, %%f%d\n", rs, rt);
  if (fesetround(FE_TONEAREST) == 0) {
    fprintf(stderr, "Failed to set rounding mode.\n");
    exit(EXIT_FAILURE);
  }
  int32_t res = (int) nearbyintf(load_float(rt));
  RB[rt] = res;
  dbg_printf("Result = %d\n", res);
}

void ac_behavior(ceilwd) {
  dbg_printf("ceil.w.d %%%d, %%f%d\n", rs, rt);
  int32_t res = (int) ceil(load_double(rt));
  RB[rt] = res;
  dbg_printf("Result = %d\n", res);
}

void ac_behavior(ceilws) {
  dbg_printf("ceil.w.s %%%d, %%f%d\n", rs, rt);
  int32_t res = (int) ceilf(load_float(rt));
  RB[rt] = res;
  dbg_printf("Result = %d\n", res);
}

void ac_behavior(floorwd) {
  dbg_printf("floor.w.d %%%d, %%f%d\n", rs, rt);
  int32_t res = (int) floor(load_double(rt));
  RB[rt] = res;
  dbg_printf("Result = %d\n", res);
}

void ac_behavior(floorws) {
  dbg_printf("floor.w.s %%%d, %%f%d\n", rs, rt);
  int32_t res = (int) floorf(load_float(rt));
  RB[rt] = res;
  dbg_printf("Result = %d\n", res);
}

void ac_behavior(stwl) {
  dbg_printf("stwl r%d, %d(r%d)\n", rt, imm, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (addr & 0x3) * 8;
  data = RB[rt];
  data >>= offset;
  data |= DATA_PORT->read(addr & 0xFFFFFFFC) & (0xFFFFFFFF << (32 - offset));
  DATA_PORT->write(addr & 0xFFFFFFFC, data);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + imm, rt);
  dbg_printf("Result = %#x\n", data);
}

void ac_behavior(stwr) {
  dbg_printf("stwr r%d, %d(r%d)\n", rt, imm, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (3 - (addr & 0x3)) * 8;
  data = RB[rt];
  data <<= offset;
  data |= DATA_PORT->read(addr & 0xFFFFFFFC) & ((1 << offset) - 1);
  DATA_PORT->write(addr & 0xFFFFFFFC, data);
  dbg_printf("* MEM[%x] <= r%d\n", RB[rs] + imm, rt);
  dbg_printf("Result = %#x\n", data);
}

void ac_behavior(addi) {
  dbg_printf("addi r%d, r%d, %d\n", rt, rs, imm);
  RB[rt] = RB[rs] + imm;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(slti) {
  dbg_printf("slti r%d, r%d, %d\n", rt, rs, imm);
  // Set the RD if RS< IMM
  if ((ac_Sword)RB[rs] < (ac_Sword)imm)
    RB[rt] = 1;
  // Else reset RD
  else
    RB[rt] = 0;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(sltiu) {
  dbg_printf("sltiu r%d, r%d, %d\n", rt, rs, imm & 0x3FFF);
  // Set the RD if RS< IMM
  if ((ac_Uword)RB[rs] < (ac_Uword)(imm & 0x3FFF))
    RB[rt] = 1;
  // Else reset RD
  else
    RB[rt] = 0;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(andi) {
  dbg_printf("andi r%d, r%d, %d\n", rt, rs, imm & 0x3FFF);
  RB[rt] = RB[rs] & (imm & 0x3FFF);
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(ori) {
  dbg_printf("ori r%d, r%d, %d\n", rt, rs, imm & 0x3FFF);
  RB[rt] = RB[rs] | (imm & 0x3FFF);
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(xori) {
  dbg_printf("xori r%d, r%d, %d\n", rt, rs, imm & 0x3FFF);
  RB[rt] = RB[rs] ^ (imm & 0x3FFF);
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(add) {
  dbg_printf("add r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] + RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(sub) {
  dbg_printf("sub r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] - RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(slt) {
  dbg_printf("slt r%d, r%d, r%d\n", rd, rs, rt);
  // Set the RD if RS< RT
  if ((ac_Sword)RB[rs] < (ac_Sword)RB[rt])
    RB[rd] = 1;
  // Else reset RD
  else
    RB[rd] = 0;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(sltu) {
  dbg_printf("sltu r%d, r%d, r%d\n", rd, rs, rt);
  // Set the RD if RS < RT
  if (RB[rs] < RB[rt])
    RB[rd] = 1;
  // Else reset RD
  else
    RB[rd] = 0;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(instr_and) {
  dbg_printf("and r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] & RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(instr_or) {
  dbg_printf("or r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] | RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(instr_xor) {
  dbg_printf("xor r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] ^ RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(instr_nor) {
  dbg_printf("nor r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = ~(RB[rs] | RB[rt]);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(shl) {
  dbg_printf("shl r%d, r%d, %d\n", rd, rt, rs);
  RB[rd] = RB[rt] << rs;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(shr) {
  dbg_printf("shr r%d, r%d, %d\n", rd, rt, rs);
  RB[rd] = RB[rt] >> rs;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(asr) {
  dbg_printf("asr r%d, r%d, %d\n", rd, rt, rs);
  RB[rd] = (ac_Sword)RB[rt] >> rs;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(shlr) {
  dbg_printf("shlr r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = RB[rt] << (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(shrr) {
  dbg_printf("shrr r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = RB[rt] >> (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(asrr) {
  dbg_printf("asrr r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = (ac_Sword)RB[rt] >> (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(mul) {
  dbg_printf("mul r%d, r%d, r%d, r%d\n", rv, rd, rs, rt);

  long long result;
  int half_result;

  result = (ac_Sword)RB[rs];
  result *= (ac_Sword)RB[rt];

  half_result = (result & 0xFFFFFFFF);
  if (rd != 0)
    RB[rd] = half_result;

  half_result = ((result >> 32) & 0xFFFFFFFF);
  if (rv != 0)
    RB[rv] = half_result;

  dbg_printf("Result = %#llx\n", result);
}

void ac_behavior(mulu) {
  dbg_printf("mulu r%d, r%d, r%d, r%d\n", rv, rd, rs, rt);

  unsigned long long result;
  int half_result;

  result = (ac_Uword)RB[rs];
  result *= (ac_Uword)RB[rt];

  half_result = (result & 0xFFFFFFFF);
  if (rd != 0)
    RB[rd] = half_result;

  half_result = ((result >> 32) & 0xFFFFFFFF);
  if (rv != 0)
    RB[rv] = half_result;

  dbg_printf("Result = %#llx\n", result);
}

void ac_behavior(div) {
  dbg_printf("div r%d, r%d, r%d, r%d\n", rv, rd, rs, rt);
  if (rd != 0)
    RB[rd] = (ac_Sword)RB[rs] / (ac_Sword)RB[rt];
  if (rv != 0)
    RB[rv] = (ac_Sword)RB[rs] % (ac_Sword)RB[rt];
}

void ac_behavior(divu) {
  dbg_printf("divu r%d, r%d, r%d, r%d\n", rv, rd, rs, rt);
  if (rd != 0)
    RB[rd] = RB[rs] / RB[rt];
  if (rv != 0)
    RB[rv] = RB[rs] % RB[rt];
}

void ac_behavior(jump) {
  dbg_printf("jump %d\n", laddr);
  if (laddr == 0) {
    printf("Jump to address zero\n");
    exit(-1);
  }
  laddr = laddr << 2;
  ac_pc = (ac_pc & 0xF0000000) | laddr;
  dbg_printf("Target = %#x\n", (ac_pc & 0xF0000000) | laddr);
}

void ac_behavior(call) {
  dbg_printf("call %d\n", addr);
  RB[Ra] = ac_pc;

  addr = addr << 2;
  ac_pc = (ac_pc & 0xF0000000) | addr;

  dbg_printf("Target = %#x\n", (unsigned)ac_pc);
  dbg_printf("Return = %#x\n", RB[Ra]);
}

void ac_behavior(jumpr) {
  dbg_printf("jumpr r%d\n", rt);
  ac_pc = RB[rt];
  dbg_printf("Target = %#x\n", RB[rt]);
}

void ac_behavior(callr) {
  dbg_printf("callr r%d\n", rt);
  RB[Ra] = ac_pc;
  ac_pc = RB[rt];
  dbg_printf("Target = %#x\n", RB[rt]);
  dbg_printf("Return = %#x\n", ac_pc + 4);
}

void ac_behavior(jeq) {
  dbg_printf("jeq r%d, r%d, %d\n", rs, rt, imm);
  if (RB[rs] == RB[rt]) {
    ac_pc = ac_pc + (imm << 2);
    dbg_printf("Taken to %#x\n", (unsigned)ac_pc);
  }
}

void ac_behavior(jne) {
  dbg_printf("jne r%d, r%d, %d\n", rs, rt, imm);
  if (RB[rs] != RB[rt]) {
    ac_pc = ac_pc + (imm << 2);
    dbg_printf("Taken to %#x\n", (unsigned)ac_pc);
  }
}

void ac_behavior(jlez) {
  dbg_printf("jlez r%d, %d\n", rt, imm);
  if ((RB[rt] == 0) || (RB[rt] & 0x80000000)) {
    ac_pc = ac_pc + (imm << 2), 1;
    dbg_printf("Taken to %#x\n", (unsigned)ac_pc);
  }
}

void ac_behavior(jgtz) {
  dbg_printf("jgtz r%d, %d\n", rt, imm);
  if (!(RB[rt] & 0x80000000) && (RB[rt] != 0)) {
    ac_pc = ac_pc + (imm << 2);
    dbg_printf("Taken to %#x\n", (unsigned)ac_pc);
  }
}

void ac_behavior(jltz) {
  dbg_printf("jltz r%d, %d\n", rt, imm);
  if (RB[rt] & 0x80000000) {
    ac_pc = ac_pc + (imm << 2);
    dbg_printf("Taken to %#x\n", (unsigned)ac_pc);
  }
}

void ac_behavior(jgez) {
  dbg_printf("jgez r%d, %d\n", rt, imm);
  if (!(RB[rt] & 0x80000000)) {
    ac_pc = ac_pc + (imm << 2);
    dbg_printf("Taken to %#x\n", (unsigned)ac_pc);
  }
}

void ac_behavior(ldi) {
  dbg_printf("ldi r%d, %d\n", rt, imm);
  ldireg = rt;
  RB[ldireg] &= 0xFFFFC000;
  RB[ldireg] |= imm & 0x3FFF;
  dbg_printf("Result is %#x\n", RB[ldireg]);
}

void ac_behavior(sys_call) {
  uint32_t sysnum = RB[4];
  if (sysnum == 0x100C) {
    fprintf(stderr, "Warning: fstat unimplemented.\n");
    RB[2] = -1;
    return;
  }
  // relocating regs
  RB[4] = RB[5];
  RB[5] = RB[6];
  RB[6] = RB[7];
  RB[7] = RB[8];
  RB[8] = RB[9];
  dbg_printf("Syscall number: 0x%X\t(%d)\n", sysnum, sysnum);
  if (syscall.process_syscall(sysnum) == -1) {
    fprintf(stderr, "Warning: Unimplemented syscall.\n");
    fprintf(stderr, "\tCaller address: 0x%X\n\tSyscall number: 0x%X\t%d\n",
            (unsigned int)ac_pc, sysnum, sysnum);
    RB[2] = -1;
  }
  // Sets a3 to 1 or 0 for error/success
  if ((int)RB[2] < 0)
    RB[7] = 1;
  else
    RB[7] = 0;
  dbg_printf("Result = %#x\n", RB[2]);
}

void ac_behavior(instr_break) {
  fprintf(stderr, "instr_break behavior not implemented.\n");
  exit(EXIT_FAILURE);
}

void ac_behavior(seb) {
  dbg_printf("seb r%d, r%d\n", rs, rt);
  RB[rs] = sign_extend(RB[rt], 8);
  dbg_printf("Result = %#x\n", RB[rs]);
}

void ac_behavior(seh) {
  dbg_printf("seh r%d, r%d\n", rs, rt);
  RB[rs] = sign_extend(RB[rt], 16);
  dbg_printf("Result = %#x\n", RB[rs]);
}

void ac_behavior(ext) {
  dbg_printf("ext r%d, r%d, %d, %d\n", rd, rs, rv, rt);
  uint32_t lsb = rv;
  uint32_t size = rt + 1;
  RB[rd] = (RB[rs] << (32 - size - lsb)) >> (32 - size);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(ror) {
  dbg_printf("ror r%d, r%d, %d\n", rd, rt, rs);
  RB[rd] = rotate_right(RB[rt], rs);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(rorr) {
  dbg_printf("rorr r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = rotate_right(RB[rt], RB[rs]);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(clz) {
  dbg_printf("clz %%%d, %%%d\n", rs, rt);
  uint32_t x = RB[rt];
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x = ffs(x + 1);
  if (x != 0) {
    x = 32 - x + 1;
  }
  RB[rs] = x;
  dbg_printf("Result = %#x\n", x);
}

void ac_behavior(ll) {
  dbg_printf("ll r%d, %d(r%d)\n", rt, imm & 0x3FFF, rs);
  RB[rt] = DATA_PORT->read(RB[rs] + imm);
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(sc) {
  dbg_printf("sc r%d, %d(r%d)\n", rt, imm & 0x3FFF, rs);
  DATA_PORT->write(RB[rs] + imm, RB[rt]);
  RB[rt] = 1;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior(sync) { dbg_printf("sync\n"); }

void ac_behavior(teq) {
  dbg_printf("teq %%%d, %%%d\n", rs, rt);
  if (RB[rs] == RB[rt]) {
    fprintf(stderr, "Trap generated at PC=0x%X\n", (uint32_t)ac_pc);
    exit(EXIT_FAILURE);
  }
}

void ac_behavior(movz) {
  dbg_printf("movz r%d, r%d, r%d\n", rd, rs, rt);
  if (RB[rt] == 0)
    RB[rd] = RB[rs];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(movn) {
  dbg_printf("movn r%d, r%d, r%d\n", rd, rs, rt);
  if (RB[rt] != 0)
    RB[rd] = RB[rs];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior(movzd) {
  dbg_printf("movz.d %%f%d, %%f%d, %%%d\n", rd, rs, rt);
  if (RB[rt] != 0) {
    RBD[rd] = RBD[rs];
    RBD[rd + 1] = RBD[rs + 1];
  }
}

void ac_behavior(movzs) {
  dbg_printf("movz.s %%f%d, %%f%d, %%%d\n", rd, rs, rt);
  if (RB[rt] != 0) {
    RBS[rd] = RBS[rs];
  }
}

void ac_behavior(movnd) {
  dbg_printf("movn.d %%f%d, %%f%d, %%%d\n", rd, rs, rt);
  if (RB[rt] != 0) {
    RBD[rd] = RBD[rs];
    RBD[rd + 1] = RBD[rs + 1];
  }
}

void ac_behavior(movns) {
  dbg_printf("movn.s %%f%d, %%f%d, %%%d\n", rd, rs, rt);
  if (RB[rt] != 0) {
    RBS[rd] = RBS[rs];
  }
}

void ac_behavior(movf) {
  dbg_printf("movf r%d, r%d, %%fcc0\n", rs, rt);
  if (cc == 0)
    RB[rs] = RB[rt];
  dbg_printf("Result = %#x\n", RB[rs]);
}

void ac_behavior(movt) {
  dbg_printf("movt r%d, r%d, %%fcc0\n", rs, rt);
  if (cc != 0)
    RB[rs] = RB[rt];
  dbg_printf("Result = %#x\n", RB[rs]);
}

void ac_behavior(movtd) {
  dbg_printf("movt.d %%f%d, %%f%d, %%fcc0\n", rs, rt);
  if (cc != 0) {
    RBD[rs] = RBD[rt];
    RBD[rs + 1] = RBD[rt + 1];
  }
}

void ac_behavior(movts) {
  dbg_printf("movt.s %%f%d, %%f%d, %%fcc0\n", rs, rt);
  if (cc != 0) {
    RBS[rs] = RBS[rt];
  }
}

void ac_behavior(movfd) {
  dbg_printf("movf.d %%f%d, %%f%d, %%fcc0\n", rs, rt);
  if (cc == 0) {
    RBD[rs] = RBD[rt];
    RBD[rs + 1] = RBD[rt + 1];
  }
}

void ac_behavior(movfs) {
  dbg_printf("movf.s %%f%d, %%f%d, %%fcc0\n", rs, rt);
  if (cc == 0) {
    RBS[rs] = RBS[rt];
  }
}

void ac_behavior(maddd) {
  dbg_printf("madd.d %%f%d, %%f%d, %%f%d, %%f%d\n", rd, rv, rs, rt);
  double res = load_double(rs) * load_double(rt) + load_double(rv);
  save_double(res, rd);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior(madds) {
  dbg_printf("madd.s %%f%d, %%f%d, %%f%d, %%f%d\n", rd, rv, rs, rt);
  float res = load_float(rs) * load_float(rt) + load_float(rv);
  save_float(res, rd);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior(msubd) {
  dbg_printf("msub.d %%f%d, %%f%d, %%f%d, %%f%d\n", rd, rv, rs, rt);
  double res = load_double(rs) * load_double(rt) - load_double(rv);
  save_double(res, rd);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior(msubs) {
  dbg_printf("msub.s %%f%d, %%f%d, %%f%d, %%f%d\n", rd, rv, rs, rt);
  float res = load_float(rs) * load_float(rt) - load_float(rv);
  save_float(res, rd);
  dbg_printf("Result = %f\n", res);
}
