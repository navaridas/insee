/**
* @file
* @brief	Some miscellaneus tools & definitions.
*
* Tools for memory allocation & error printing.
* Definition of some useful macros.
* Definition of some enumerations.
*/

#ifndef _misc
#define _misc

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"

/**
* Choose a random number in [ 0, m ).
* 
* @param m maximum.
* @return A random number.
*/
#define ztm(m) (long) (m * ( (1.0*rand() ) / (RAND_MAX+1.0)))

/**
* Return the absolute value
* 
* @param m The number.
* @return The absolute value of m.
*/
#define abs(m) ((m<0) ? (-m) : (m))

/**
* The sign of a numeric value.
*
* For zero, the return value is 1
*/
#define sign(x) (x<0 ? -1 : 1)

#define P_NULL (-1) ///< Definition of a NULL value.

/**
* Definition of the maximum chooser
* 
* @param a One number.
* @param b Another One.
* @return The maximum of both.
*/
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

/**
* Definition of the minimum chooser
* 
* @param a One number.
* @param b Another One.
* @return The minimum of both.
*/
#ifndef min
#define min(a,b)            (((a) > (b)) ? (b) : (a))
#endif

/**
* Definition of the module of a division
* 
* @param a One number.
* @param b Another One.
* @return a mod b.
*/
#ifndef mod
#define mod(a,b)            ((((a)%(b)) <  0 ) ? ((a)%(b)) + (b) : ((a)%(b)) )
#endif

/**
* Definition of boolean values.
*/
typedef enum bool_t {
	FALSE = 0, TRUE = 1
} bool_t;

/**
* Definition of all accepted topologies.
*/
typedef enum topo_t {
	// Original fsin topologies
	TORUS, MESH, MIDIMEW, TWISTED,
	// Direct topologies
	DIRECT,
	// Other cube-based topologies
	ICUBE, CUBE,
	// Indirect Topologies
	FATTREE, SLIMTREE, THINTREE, INDIRECT,
	// File defined: TO DO
	FILEDEF
} topo_t;

/**
* Classes of VC management.
*/
typedef enum vc_management_t {
	BUBBLE_MANAGEMENT,		// Normal bubble -- requires just 1 VC
	DALLY_MANAGEMENT,		// Dally management -- requires at least 2 VC
	DOUBLE_MANAGEMENT,		// Double bubble: XY, YX. More options for 3D
	TREE_MANAGEMENT,		// Fattree, thintree, slimtree.
	ICUBE_MANAGEMENT,		// indirect cube.
} vc_management_t;

/**
* Classes of traffic sources.
*/
typedef enum traffic_pattern_t {
	UNIFORM,	// Random selection of destination
	LOCAL,  // 50% of the traffic goes to [0,1], 25% goes to [2,3], 12.5% goes to [4-7], 12.5% goes further.
	SEMI,		// Uniform traffic in the left-half of a cube-like network
	HOTREGION,	// 1/4 of traffic goes to first 1/8 of network; remaining traffic is uniform
	DISTRIBUTE,	// Starting from node n+1, sent packets to n+1, n+2, n+3, ...
	RSDIST,		// Like DISTRIBUTE, but starting with a random destination
	TRANSPOSE,	// Matrix transpose permutation
	TORNADO,	// Tornado permutation
	COMPLEMENT,	// Bit-complement permutation
	BUTTERFLY,	// Butterfly permutation CHECK CHECK CHECK
	SHUFFLE,	// Perfect shuffle permutation
	REVERSAL,	// Bit-reversal permutation
	TRACE,		// Load from trace file
	POPULATION,	// Load a population of a distribution from a trace file
	HISTOGRAM,	// load the histogram of a distribution from a trace file
	HOTSPOT		// 5% of traffic goes to the eth connection in (0,0), 95% uniform
} traffic_pattern_t;

/**
* Definition of consumption modes.
*/
typedef enum cons_mode_t {
	SINGLE_CONS, MULTIPLE_CONS
} cons_mode_t;

/**
* Definition of DOR classes.
*/
typedef enum routing_t {
	DIMENSION_ORDER_ROUTING,	// DOR means Dimesion Order Routing.
	DIRECTION_ORDER_ROUTING,	// DOR means Direction Order Routing.
	CUBE_ROUTING,				// Just for cubes.
	STATIC_ROUTING,				// Static routing for trees and indirect cube .
	ADAPTIVE_ROUTING,			// Adaptive routing for trees and indirect cube .
	IC_1M_ROUTING,              // STATIC DOR in icubes
	IC_4M_ROUTING,              // STATIC DOR in icubes (4meshes)
	MULTISTAGE_ROUTING,			// Just for multistage.
} routing_t;

/**
* Definition of all request port modes.
*/
typedef enum req_mode_t {
	// Variants for BUBBLE
ONE_OR_MORE_REQUIRED,
	BUBBLE_OBLIVIOUS_REQ, DOUBLE_OBLIVIOUS_REQ, TREE_REQ, ICUBE_REQ, 
TWO_OR_MORE_REQUIRED,
	BUBBLE_ADAPTIVE_RANDOM_REQ, BUBBLE_ADAPTIVE_SHORTEST_REQ, BUBBLE_ADAPTIVE_SMART_REQ, DOUBLE_ADAPTIVE_REQ, BIMODAL_REQ,
	// Variants for DALLY
	DALLY_TRC_REQ, DALLY_BASIC_REQ, DALLY_IMPROVED_REQ,
THREE_OR_MORE_REQUIRED,
	DALLY_ADAPTIVE_REQ,
SIX_REQUIRED,
	HEXA_ADAPTIVE_REQ, HEXA_OBLIVIOUS_REQ
} req_mode_t;

/**
* Definition of arbitration mechanism.
*/
typedef enum arb_mode_t {
	ROUNDROBIN_ARB, FIFO_ARB, LONGEST_ARB, RANDOM_ARB, AGE_ARB
} arb_mode_t;

/**
* Definition of injection techniques.
*/
typedef enum inj_mode_t {
	// No prerouting
	SHORTEST_INJ,
	// With prerouting
	DOR_INJ, DOR_SHORTEST_INJ, SHORTEST_PROFITABLE_INJ, LONGEST_PATH_INJ
} inj_mode_t;

/**
* Definition of task placement types for trace driven.
*/
typedef enum placement_t{
	CONSECUTIVE_PLACE, SHUFFLE_PLACE, RANDOM_PLACE, SHIFT_PLACE, ROW_PLACE, COLUMN_PLACE, QUADRANT_PLACE, ICUBE_PLACE, DIAGONAL_PLACE, FILE_PLACE
} placement_t;

/**
* Definition of the source type for trace driven.
*/
typedef enum source_t{
	NO_SOURCE, INDEPENDENT_SOURCE, OTHER_SOURCE
} source_t;

// Some declarations.
void * alloc(long);
void panic(char *mes);

#endif /* _misc */

