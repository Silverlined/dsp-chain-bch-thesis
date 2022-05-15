/* K=7 r=1/2 Viterbi decoder with optional Intel or PowerPC SIMD
 * Copyright Feb 2004, Phil Karn, KA9Q
 */
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "fec.h"

/* Create a new instance of a Viterbi decoder */
void *create_viterbi27(int len)
{
  find_cpu_mode();

  switch (Cpu_mode) {
  case PORT:
  default:
    return create_viterbi27_port(len);
  }
}

void set_viterbi27_polynomial(int polys[2])
{
  switch (Cpu_mode) {
  case PORT:
  default:
    set_viterbi27_polynomial_port(polys);
    break;
  }
}

/* Initialize Viterbi decoder for start of new frame */
int init_viterbi27(void *p, int starting_state)
{
  switch (Cpu_mode) {
  case PORT:
  default:
    return init_viterbi27_port(p, starting_state);
  }
}

/* Viterbi chainback */
int chainback_viterbi27(
  void *p,
  unsigned char *data, /* Decoded output data */
  unsigned int nbits, /* Number of data bits */
  unsigned int endstate)  /* Terminal encoder state */
{

  switch (Cpu_mode) {
  case PORT:
  default:
    return chainback_viterbi27_port(p, data, nbits, endstate);
  }
}

int
chainback_viterbi27_unpacked_trunc(void *p, unsigned char *data,
                                   unsigned int nbits)
{
  return chainback_viterbi27_port_unpacked_trunc(p, data, nbits);
}

/* Delete instance of a Viterbi decoder */
void delete_viterbi27(void *p)
{
  switch (Cpu_mode) {
  case PORT:
  default:
    delete_viterbi27_port(p);
    break;
  }
}

/* Update decoder with a block of demodulated symbols
 * Note that nbits is the number of decoded data bits, not the number
 * of symbols!
 */
int update_viterbi27_blk(void *p, unsigned char syms[], int nbits)
{
  if (p == NULL) {
    return -1;
  }

  switch (Cpu_mode) {
  case PORT:
  default:
    update_viterbi27_blk_port(p, syms, nbits);
    break;
  }
  return 0;
}
