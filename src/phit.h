/**
* @file
* @brief	Definition of FSIN Phits.
*/
#ifndef _phit
#define _phit

#include "misc.h"

/** 
* Type of phit.
*/
typedef enum phit_class {
	EMPTY = 0,		///< An empty phit without any data.
	RR = 1,			///< A routing record.
	SRC = 2,		///< ?????
	INFO = 3,		///< A Phit containing information
	TAIL = 4,		///< This Phit is the last of a packet
	RR_TAIL = 5		///< This phit is a full packet itself.
} phit_class;

/**
* Structure containing a phit.
*/
typedef struct phit {
	phit_class pclass;		///< The type of the phit. @see phit_class
	unsigned long packet;	///< The id of the packet. @see pkt_mem.c
} phit;
#endif /* _phit */

