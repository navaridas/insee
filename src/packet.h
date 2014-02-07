/**
* @file
* @brief	Definition of FSIN Packets.
*/
#ifndef _packet
#define _packet

#include "misc.h"

/**
* Definition of a routing record.
*/
typedef struct routing_r {
	long *rr;
	long size;
} routing_r;


#if (BIMODAL_SUPPORT != 0)
/**
* Message type for bimodal traffic.
*/
typedef enum message_l {
	SHORT_MSG,		///< 1 packet length message.
	LONG_MSG,		///< message with longmessages packets length.
	LONG_LAST_MSG	///< last packet of a long message.
} message_l;
#endif /* BIMODAL*/

/**
* Structure that defines a FSIN packet.
*
* It contains all the needed information for routing, stats calculating
* and bimodal & trace traffic supporting.
*/
typedef struct packet_t {
	CLOCK_TYPE tt;		///< Timestamp
	long to;		///< Destiny
	long from;		///< Origin
	long size;		///< Size ( in phits )
	routing_r rr;	///< Routing record (rz not used for Midimew)
	CLOCK_TYPE inj_time;	///< Cycle in wich a packet has been injected to the network.
	long n_hops;	///< Hops until current position.

#if (BIMODAL_SUPPORT != 0)
	message_l mtype;///< Type of message in bimodal injection.
#endif /* BIMODAL */
#if (TRACE_SUPPORT != 0)
	long task;		///< Task id in event driven simulation
	long length;	///< Length of a message in event driven simulation
#endif /* TRACE */
#if (EXECUTION_DRIVEN != 0)
	long id_trama;	///< Identifier of an Ethernet frame for execution-driven simulation
		/* Este campo es el identificador tanto de paquete dentro */
		/* de un mismo mensaje como de phit dentro del paquete */
		/* Formato de id_trama: XXX...XYYY...YY */
		/* Donde las X representan los bits que forman la secuencia de la trama */
		/* ethernet, y las Y representan la secuencia del paquete al que */
		/* pertenece el phit. Una trama ethernet se divide en varios paquetes */
		/* El numero de bits que ocupa la secuencia del paquete es: */
		/* round_up(log(1500/(packet_size_in_phits*phit_size))/log 2) */
#endif

} packet_t;

#endif /* _packet */
