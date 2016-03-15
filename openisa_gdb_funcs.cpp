#include "openisa.H"

// 'using namespace' statement to allow access to all
// openisa-specific datatypes
using namespace openisa_parms;

int openisa::nRegs(void) {
   return 73;
}


ac_word openisa::reg_read( int reg ) {
  /* general purpose registers */
  if ( ( reg >= 0 ) && ( reg < 32 ) )
    return RB.read( reg );
  else {
    if (reg == 33)
      return lo;
    else if (reg == 34)
      return hi;
    else
      /* pc */
      if ( reg == 37 )
        return ac_pc;
  }

  return 0;
}


void openisa::reg_write( int reg, ac_word value ) {
  /* general purpose registers */
  if ( ( reg >= 0 ) && ( reg < 32 ) )
    RB.write( reg, value );
  else
    {
      /* lo, hi */
      if ( ( reg >= 33 ) && ( reg < 35 ) )
        RB.write( reg - 1, value );
      else
        /* pc */
        if ( reg == 37 )
          ac_pc = value;
    }
}


unsigned char openisa::mem_read( unsigned int address ) {
  return IM->read_byte( address );
}


void openisa::mem_write( unsigned int address, unsigned char byte ) {
  IM->write_byte( address, byte );
}
