/**
* @file
* @brief	Declaration of FSIN routers.
*/
#ifndef _router
#define _router

#include "globals.h"

/**
* Calculates the port asigned to a direction pair (dimension, way ).
* 
* @param j dimesion.
* @param k way.
*/
#define dir(j,k) ((j*nways) + k)

/**
* Given a port and VC, returns channel identifier.
* 
* @param p port.
* @param c VC.
*/
#define port_address(p,c) ((p*nchan) + c)

/**
* Given the coordinates X,Y,Z of a node, returns the node identifier. Only valid for mesh-like topologies.
*/
#define address(cx,cy,cz) (cx + cy*nodes_x + cz*nodes_x*nodes_y)

#define ESCAPE 0	///< The Escape VC is always #0
#define NULL_PORT -1	///< A way to denote "no port"
#define NULL_PACKET 0xffffffff	///< A way to denote "no packet"

/**
* An enumeration to define dimensions X, Y and Z channels. When used with ports,
* possible values are also INJ (injection) and CON (consuption).
*/
typedef enum dim {
	D_X = 0,
	D_Y = 1,
	D_Z = 2,
	INJ = 3,
	CON = 4
} dim;

/**
* An enumeration to define multistage coordinates
*/
typedef enum multistages {
	STAGE = 0,
	POSITION = 1
} multistages;

/**
* An enumeration to define the "UP" or "+" way, and the "DOWN" or "-" way
*/
typedef enum way {
	UP = 0,		///< In X: left -> right; in Y, down -> top; increasing values
	DOWN = 1	///< In X: left <- right; in Y, down <- top; decreasing values
} way;

/**
* A type to enumerate channels -- actually a long
*/
typedef long channel;

/**
* An type, related to dim, used to bet in which direction I'll try to make
* a request for reservation of a VC; it includes "-1" to denote the Escape VC
*/
typedef enum bet_type {
	B_ESCAPE = -1,		// The Escape VC
	B_TRIAL_0 = D_X,	// An adaptive VC in the X direction
	B_TRIAL_1 = D_Y,	// An adaptive VC in the Y direction
	B_TRIAL_2 = D_Z		// An adaptive VC in the Z direction
} bet_type;

/**
* A type to enumerate ports -- actually a long
*/
typedef long port_type;

/**
* Structure that defines a pair of input - output ports.
*/
typedef struct port {
	// Input section
	queue q;		///< Associated queue -- useless for consumption
	bet_type bet;	///< Which output port will I try to reserve?
	port_type aop;	///< Assigned output port for this queue
	CLOCK_TYPE tor;		///< Time of last request for output

	// Output section
	CLOCK_TYPE *req;		///< Table of requests
	port_type ri;	///< Last request attended
	port_type sip;	///< Input port using this output port

	// Others
	CLOCK_TYPE * histo;		///< size = MAX_QUEUE_LEN
	CLOCK_TYPE utilization;	///< Utilization of this port
	bool_t faulty;		///< Is there any problem with the link
} port;

/**
* Structure that defines a network router. Includes input buffer, transit queues
* and many auxiliary data structures.
*/
typedef struct router {
	// General info
	long rcoord[3];	///< Stores the router coordinates X,Y,Z (only used in dally CV management)
	long * nbor;	///< The id's of neighbors
	long * nborp;	///< The id's of neighbors' ports

	// Ports
	port * p;		///< All the node's ports
	long * op_i;	///< Indices to assign output port

	// Injection
	inj_queue * qi;				///< All the node's injectors
	port_type injecting_port;	///< Port that is injecting, when all them share a physical injection channel
	port_type next_port;		///< Used to indicate the next injector to be used, when many available
	packet_t saved_packet;		///< Packet awaiting to be injected
	long pending_packet;		///< Number of packets awaiting
	long triggered;				///< Number of packets triggered by incoming packets - Reactive traffic.

#if (PCOUNT!=0)
	/**
	* Total phits within the router.
	* If this value is 0 the router ports wont be checked to for requesting, arbitrating or moving.
	*/
	long pcount;
#endif

	// Congestion with timeouts.
	CLOCK_TYPE timeout_counter;	///< This counts the number of cycles a packet is in the router or the number of cycles without a new packet arrival.
	unsigned long timeout_packet;	///< This is the packet that we are looking to.

	bool_t congested;				///< Has this router detected congestion?

	source_t source;           ///< The source type. May be independent, no source or other.
	
	// Ports and injectors
#if (TRACE_SUPPORT == 1)
	event_q events;		///< A Queue with events to occur
	event_l occurs;	///< Lists with occurred events (one for each messsage source)
#endif /* TRACE */

#if (TRACE_SUPPORT > 1)
	event_q events;		///< A Queue with events to occur
	event_l *occurs;	///< Lists with occurred events (one for each messsage source)
#endif /* TRACE */
} router;
#endif /* _router */


