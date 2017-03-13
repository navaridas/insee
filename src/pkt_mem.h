/**
* @file
* @brief	Declaration of FSIN functions for packet management.
*/

#ifndef _pkt_mem
#define _pkt_mem

void pkt_init();
void free_pkt(unsigned long n);
unsigned long get_pkt();

#endif /* _pkt_mem */

