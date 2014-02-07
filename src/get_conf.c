/**
* @file
* @brief	Gets the configuration of the Simulation.
*
* First take default values for the simulation.
* Second take the values from the file 'fsin.conf'.
* Finally take the command line arguments.

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, A. Gonzalez, J. Navaridas

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "misc.h"
#include "globals.h"
#include "literal.h"

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif

/** The Configuration file. */
#define DEFAULT_CONF_FILE "fsin.conf"

double bg_load;

// some declarations.
static void set_default_conf(void);
static void get_option(char *);
static void get_conf_file(char *);
static void verify_conf(void);

/**
* Default values for options are specified here.
* @see literal.c
*/
static literal_t options_l[] = {
	{ 2,  "background"},	/* load for background traffic*/
	{ 2,  "bg_load"},
	{ 2,  "bg"},
	{ 3,  "plength"},	/* Packet length */
	{ 4,  "tpattern"},	/* Traffic pattern */
	{ 5,  "load"},		/* Load */
	{ 6,  "topo"},		/* Topology */
	{ 7,  "nodes_x"},	/* Nodes per dimension: X. */
	{ 8,  "nodes_y"},	/* Nodes per dimension: Y. */
	{ 9,  "nodes_z"},	/* Nodes per dimension: Z. */
	{ 10, "vc"},		/* VC management */
	{ 11, "routing"},	/* Oblivious routing */
	{ 12, "rmode"},		/* Request mode */
	{ 13, "bsize"},		/* Bubble size */
	{ 13, "bub"},		/* Bubble size */
	{ 14, "amode"},		/* Arbitration mode */
	{ 15, "cmode"},		/* Consumption mode */
	{ 16, "plevel"},	/* Print level */
	{ 17, "pheaders"},	/* Partial headers */
	{ 18, "bheaders"},	/* Batch headers */
	{ 19, "rnds" },		/* Random seed */
	{ 19, "rseed" },	/* Random seed */
	{ 20, "tql"},		/* Transit queue length */
	{ 21, "iql"},		/* Injection queue length */
	{ 22, "output"},    /* Path to the ouput files. */
	{ 23, "tracefile"}, /* the path to the trace file */
	{ 24, "pinterval"},	/* Interval to calculate intermediate results and print partial info */
	{ 25, "nways"},		/* Number of ways 1=unidri, 2=bidirectional */
	{ 26, "nchan"},		/* Number of virtual channels */
	{ 27, "ninj"},		/* Number of injectors */
	{ 28, "timeout_upper_limit"},
	{ 29, "timeout_lower_limit"},
	{ 30, "extract"},	/* Should the system extract packets at head of inj. queues when they cannot be injected? */
	{ 31, "monitored"},	/* Define the monitored node, to extract additional data from */
	{ 32, "imode"},		/* Injection mode */
	{ 33, "intransit_pr"},	/* Priority to give to packets in transit (against new traffic) */
	{ 33, "ipr"},			/* Priority to give to packets in transit (against new traffic) */
	{ 34, "drop_packets"},	/* Should the system drop packets when the cannot be injected, or wait for a better moment */
	{ 34, "drop"},
	{ 35, "bub_to_adap"},	/* Apply bubble restrictions even to adaptive channels */
	{ 35, "lbr"},	/* Apply bubble restrictions even to adaptive channels */
	{ 36, "par_inj"},	/* Parallel vs. serial injection */
	{ 37, "shotmode"},	/* Run in "shot by shot" mode a.k.a. burst-mode*/
	{ 38, "shotsize"},	/* shot size */
	{ 39, "update_period"},	/* update period to compute-distribute global buffer utilization */
	{ 40, "global_cc"},	/* global congestion control */
	{ 41, "sk_xy"},	/* skews for twisted torus  -- SEE LATER FOR MORE*/
	{ 42, "sk_xz"},
	{ 43, "sk_yx"},
	{ 44, "sk_yz"},
	{ 45, "sk_zx"},
	{ 46, "sk_zy"},
	{ 47, "warm_up_period"},
	{ 48, "conv_period"},
	{ 49, "conv_thres"},
	{ 50, "max_conv_time"},
	{ 51, "nsamples"},
	{ 51, "numshots"},
	{ 52, "sample_size"},
	{ 53, "min_batch_pkt"},
	{ 54, "placement"},
	{ 55, "longmessages"},
	{ 56, "lng_msg_ratio"},
	{ 58, "trigger_rate"},
	{ 59, "triggered"},
	{ 60, "faults"},
	{ 100, "fsin_cycle_relation"},
	{ 101, "simics_cycle_relation"},
	{ 103, "serv_addr"},
	{ 104, "num_wait_periods"},
	{ 104, "num_periodos_espera"},
	LITERAL_END
};

/**
* All the traffic patterns are specified here.
* @see literal.c
*/
literal_t pattern_l[] = {
	{ UNIFORM,		"uniform"},
	{ UNIFORM,		"random"},
	{ LOCAL,		"local"},
	{ SEMI,			"semi"},
	{ TRANSPOSE,	"transpose"},
	{ DISTRIBUTE,	"distribute"},
	{ HOTREGION,	"hotregion"},
	{ TORNADO,		"tornado"},
	{ RSDIST,		"rsdist"},
	{ COMPLEMENT,	"complement"},
	{ BUTTERFLY,	"butterfly"},
	{ SHUFFLE,		"shuffle"},
	{ REVERSAL,		"reversal"},
	{ TRACE,		"trace"},
	{ POPULATION,	"population"},
	{ POPULATION,	"pop"},
	{ HISTOGRAM,	"histogram"},
	{ HISTOGRAM,	"hist"},
	{ HOTSPOT,		"hotspot"},
	LITERAL_END
};

/**
* All the topologies allowed are specified here.
* @see literal.c
*/
literal_t topology_l[] = {
	{ MIDIMEW,	"midimew"},
	{ TWISTED,	"ttorus"},
	{ TORUS,	"torus"},
	{ MESH,		"mesh"},
	{ FATTREE,	"fattree"},
	{ FATTREE,	"fat"},
	{ THINTREE,	"thintree"},
	{ THINTREE,	"thin"},
	{ SLIMTREE,	"slimtree"},
	{ SLIMTREE,	"slim"},
	{ SLIMTREE,	"slendertree"},
	{ SLIMTREE,	"slender"},
	{ ICUBE,	"tricube"},
	{ ICUBE,	"icube"},
	LITERAL_END
};

/**
* All the Virtual channel managements are specified here.
* @see literal.c
*/
literal_t vc_l[] = {
	{ BUBBLE_MANAGEMENT,	"bubble"},
	{ DOUBLE_MANAGEMENT,	"double"},
	{ DALLY_MANAGEMENT,		"dally"},
	{ TREE_MANAGEMENT,		"tree"},
	{ ICUBE_MANAGEMENT,	"icube"},
	LITERAL_END
};

/**
* All the routing arbitration modes are specified here.
* @see literal.c
*/
literal_t routing_l[] = {
	{ DIMENSION_ORDER_ROUTING,	 "dim"},
	{ DIRECTION_ORDER_ROUTING,	 "dir"},
	{ STATIC_ROUTING,		  "static"},
	{ ADAPTIVE_ROUTING,		"adaptive"},
	{ IC_1M_ROUTING,		   "1mesh"},
	{ IC_4M_ROUTING,		   "4mesh"},
	LITERAL_END
};

/**
* All the port requesting modes are specified here.
* @see literal.c
*/
literal_t rmode_l[] = {
	{ BUBBLE_ADAPTIVE_RANDOM_REQ,	"random"},
	{ BUBBLE_ADAPTIVE_SHORTEST_REQ,	"shortest"},
	{ BUBBLE_ADAPTIVE_SMART_REQ,	"smart"},
	{ BUBBLE_OBLIVIOUS_REQ,			"oblivious"},
	{ DOUBLE_OBLIVIOUS_REQ,			"doubleob"},
	{ DOUBLE_ADAPTIVE_REQ,			"doubleadap"},
	{ HEXA_OBLIVIOUS_REQ,			"hexaob"},
	{ HEXA_ADAPTIVE_REQ,			"hexaadap"},
	{ DALLY_TRC_REQ,				"trc"},
	{ DALLY_BASIC_REQ,				"basic"},
	{ DALLY_IMPROVED_REQ,			"improved"},
	{ DALLY_ADAPTIVE_REQ,			"adaptive"},
	{ BIMODAL_REQ,					"bimodal"},
	{ TREE_REQ,						"trees"},
	{ ICUBE_REQ,					"icube"},
	LITERAL_END
};

/**
* All the arbitration modes are specified here.
* @see literal.c
*/
literal_t atype_l[] = {
	{ ROUNDROBIN_ARB,	"rr"},
	{ FIFO_ARB,			"fifo"},
	{ LONGEST_ARB,		"longest"},
	{ RANDOM_ARB,		"random"},
	{ AGE_ARB,			"oldest"},
	{ AGE_ARB,			"age"},
	LITERAL_END
};

/**
* All the consumption modes are specified here.
* @see literal.c
*/
literal_t ctype_l[] = {
	{ SINGLE_CONS,	"single"},
	{ MULTIPLE_CONS,"multiple"},
	LITERAL_END
};

/**
* All the injection modes are specified here.
* @see literal.c
*/
literal_t injmode_l[] = {
	{ SHORTEST_INJ,				"shortest"},
	{ DOR_INJ,					"dor"},
	{ DOR_SHORTEST_INJ,			"dsh"},
	{ SHORTEST_PROFITABLE_INJ,	"shp"},
	{ LONGEST_PATH_INJ,			"lpath"},
	LITERAL_END
};

/**
* All the placement strategies are specified here.
* @see literal.c
*/
literal_t placement_l[] = {
	{ CONSECUTIVE_PLACE,	"consecutive"},
	{ SHUFFLE_PLACE,		"shuffle"},
	{ RANDOM_PLACE,			"random"},
	{ SHIFT_PLACE,			"shift"},
	{ COLUMN_PLACE,			"column"},
	{ ROW_PLACE,			"row"},
	{ QUADRANT_PLACE,		"quadrant"},
	{ DIAGONAL_PLACE,		"diagonal"},
	{ ICUBE_PLACE,			"icube"},
	{ FILE_PLACE,			"file"},
	LITERAL_END
};

/**
* Gets the configuration defined into a file.
* @param fname The name of the file containing the configuration.
*/
void get_conf_file(char * fname) {
	FILE * fdesc;
	char buffer[1024];

	if((fdesc = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "WARNING: config file %s not found in current directory\n", fname);
		return;
	}

	while(fgets(buffer, 1024, fdesc) != NULL)
		if(buffer[0] != '\n' && buffer[0] != '#') {
			if(buffer[strlen(buffer) - 1] == '\n')
				buffer[strlen(buffer) - 1] = '\0';
			get_option(buffer);
		}
	fclose(fdesc);
}

/**
* Get the configuration for the simulation.
*
* First take default values for the simulation.
* Second take the values from the file 'fsin.conf'.
* Finally take the command line arguments.
* @param argn The number of arguments.
* @param args The arguments taken from the command line.
*/
void get_conf(long argn, char ** args) {
	long i;

	set_default_conf();
	get_conf_file(DEFAULT_CONF_FILE);
	for(i = 0; i < argn; ++i)
		get_option(args[i]);
	verify_conf();
}

/**
* Gets an option & its value.
*
* Put the value to the FSIN variable.
* @param option The string which contains an option=value
*/
void get_option(char * option) {
	long opt;
	char * name;
	char * value;
	char * param;
	char * sep=" _";
	char message[100];

	name = strtok(option, "=");
	if(!literal_value(options_l, name, &opt)) {
		sprintf(message, "get_option: Unknown option %s", name);
		panic(message);
	}
	value = strtok(NULL, "=");
	switch(opt) {
	case 2:
		sscanf(value, "%lf", &bg_load);
		break;
	case 3:
		param = strtok(value, sep);
		if (param)
			pkt_len = atoi(param);
		param = strtok(NULL, sep);
		if (param)
			phit_size = atoi(param);
		break;
	case 4:
		if(!literal_value(pattern_l, value, (long *) &pattern))
			panic("get_conf: Unknown traffic pattern");
		break;
	case 5:
		sscanf(value, "%lf", &load);
		break;
	case 6:
		param = strtok(value, sep);
		if(!literal_value(topology_l, param, (long *) &topo))
			panic("get_conf: Unknown topology");
		if(topo<DIRECT){
			param = strtok(NULL, sep);
			if (param) {
				nodes_x = atoi(param);
				ndim = 1;
			}
			param = strtok(NULL, sep);
			if (param) {
				nodes_y = atoi(param);
				ndim = 2;
			}
			param = strtok(NULL, sep);
			if (param) {
				nodes_z = atoi(param);
				ndim = 3;
			}
		}
		else if (topo==FATTREE){
			param = strtok(NULL, sep);
			if (param) radix = atoi(param);
			param = strtok(NULL, sep);
			if (param) nstages = atoi(param);
			stDown=stUp=radix;
			radix=2*radix;
		}
		else if (topo==SLIMTREE || topo==THINTREE){
			param = strtok(NULL, sep);
			if (param) stDown = atoi(param);
			param = strtok(NULL, sep);
			if (param) stUp = atoi(param);
			param = strtok(NULL, sep);
			if (param) nstages = atoi(param);
			radix = stDown+stUp;
		}
		else if (topo==ICUBE){
			param = strtok(NULL, sep);
			if (param) {
				stDown = nodes_per_switch = atoi(param);
			}
			param = strtok(NULL, sep);
			if (param) {
				links_per_direction = atoi(param);
			}
			param = strtok(NULL, sep);
			if (param) {
				nodes_x = atoi(param);
				ndim=1;
			}
			param = strtok(NULL, sep);
			if (param) {
				nodes_y = atoi(param);
				ndim=2;
			}
			else nodes_y=1;
			param = strtok(NULL, sep);
			if (param) {
				nodes_z = atoi(param);
				ndim=3;
			}
			else
				nodes_z=1;
		}
		break;
	case 7:
	case 8:
	case 9:
		panic("The Correct Syntax is now \"topo=<direct_topology>_<nodes_x>_<nodes_y>_<nodes_z>\" ");
		break;
	case 10:
		if(!literal_value(vc_l, value, (long *) &vc_management))
			panic("get_conf: Unknown vc management");
		break;
	case 11:
		if(!literal_value(routing_l, value, (long *) &routing))
			panic("get_conf: Unknown routing");
		break;
	case 12:
		if(!literal_value(rmode_l, value, (long *) &req_mode))
			panic("get_conf: Unknown request mode");
		break;
	case 13:
		param = strtok(value, sep);
		bub_x = bub_y = bub_z = 2;
		if (param)
			bub_y = bub_z = bub_x = atoi(param);
		param = strtok(NULL, sep);
		if (param)
			bub_z = bub_y = atoi(param);
		param = strtok(NULL, sep);
		if (param)
			bub_z = atoi(param);
		break;
	case 14:
		if(!literal_value(atype_l, value, (long *) &arb_mode))
			panic("get_conf: Unknown arbitration mode");
		break;
	case 15:
		if(!literal_value(ctype_l, value, (long *) &cons_mode))
			panic("get_conf: Unknown consumption mode");
		break;
	case 16:
		sscanf(value, "%ld", &plevel);
		break;
	case 17:
		sscanf(value, "%ld", &pheaders);
		if (pheaders>2047)
			panic("get_conf: Invalid partial header value");
		break;
	case 18:
		sscanf(value, "%ld", &bheaders);
		if (pheaders>8191)
			panic("get_conf: Invalid batch header value");
		break;
	case 19:
		sscanf(value, "%ld", &r_seed);
		break;
	case 20:
		sscanf(value, "%ld", &buffer_cap);
		break;
	case 21:
		sscanf(value, "%ld", &binj_cap);
		break;
	case 22:
		sscanf(value, "%s", &file);
		break;
	case 23:
		sscanf(value, "%s", &trcfile);
		break;
	case 24:
		sscanf(value, "%" SCAN_CLOCK, &pinterval);
		break;
	case 25:
		sscanf(value, "%ld", &nways);
		break;
	case 26:
		sscanf(value, "%ld", &nchan);
		break;
	case 27:
		sscanf(value, "%ld", &ninj);
		break;
	case 28:
		sscanf(value, "%" SCAN_CLOCK, &timeout_upper_limit);
		break;
	case 29:
		sscanf(value, "%" SCAN_CLOCK, &timeout_lower_limit);
		break;
	case 30:
		sscanf(value, "%ld", &extract);
		break;
	case 31:
		sscanf(value, "%ld", &monitored);
		break;
	case 32:
		if(!literal_value(injmode_l, value, (long *) &inj_mode))
			panic("get_conf: Unknown injection mode");
		break;
	case 33:
		sscanf(value, "%lf", &intransit_pr);
		break;
	case 34:
		sscanf(value, "%d", &drop_packets);
		if (drop_packets)
			drop_packets = TRUE;
		break;
	case 35:
		sscanf(value, "%ld", &bub_adap[1]);
		break;
	case 36:
		sscanf(value, "%d", &parallel_injection);
		if (parallel_injection)
			parallel_injection = TRUE;
		break;
	case 37:
		sscanf(value, "%d", &shotmode);
		if (shotmode)
			shotmode = TRUE;
		break;
	case 38:
		sscanf(value, "%ld", &shotsize);
		break;
	case 39:
		sscanf(value, "%" SCAN_CLOCK, &update_period);
		break;
	case 40:
		sscanf(value, "%lf", &global_cc);
		break;
	case 41:
		sscanf(value, "%ld", &sk_xy);
		break;
	case 42:
		sscanf(value, "%ld", &sk_xz);
		break;
	case 43:
		sscanf(value, "%ld", &sk_yx);
		break;
	case 44:
		sscanf(value, "%ld", &sk_yz);
		break;
	case 45:
		sscanf(value, "%ld", &sk_zx);
		break;
	case 46:
		sscanf(value, "%ld", &sk_zy);
		break;
	case 47:
		sscanf(value, "%" SCAN_CLOCK, &warm_up_period);
		break;
	case 48:
		sscanf(value, "%" SCAN_CLOCK, &conv_period);
		break;
	case 49:
		sscanf(value, "%lf", &threshold);
		break;
	case 50:
		sscanf(value, "%" SCAN_CLOCK, &max_conv_time);
		break;
	case 51:
		sscanf(value, "%d", &samples);
		break;
	case 52:
		sscanf(value, "%" SCAN_CLOCK, &batch_time);
		break;
	case 53:
		sscanf(value, "%d", &min_batch_size);
		break;
	case 54:
		param = strtok(value, sep);
		if(!literal_value(placement_l, param, (long *) &placement))
			panic("get_conf: Unknown placement mode");
		if (placement==SHIFT_PLACE){
			param = strtok(NULL, sep);
			if (param)
				shift = atoi(param);
			else
				shift=0;
		}
        if (placement==ICUBE_PLACE){
			param = strtok(NULL, sep);
			if (param)
				pnodes_x = atoi(param);
			else
				pnodes_x = 1;
   			param = strtok(NULL, sep);
			if (param)
				pnodes_y = atoi(param);
			else
				pnodes_y = 1;
			param = strtok(NULL, sep);
			if (param)
				pnodes_z = atoi(param);
			else
				pnodes_z = 1;
		}
		if (placement==FILE_PLACE){
			param = strtok(NULL, sep);
			if (param)
				strcpy(placefile, param);
			else
				panic("placement from file requires a placement file");
		}
		param = strtok(NULL, sep);
		if (param)
			trace_nodes = atoi(param);
		else
			trace_nodes = 0;
		param = strtok(NULL, sep);
		if (param)
			trace_instances = atoi(param);
		else
			trace_instances=0;
		break;
	case 55:
#if (BIMODAL_SUPPORT != 0)
		sscanf(value, "%d", &msglength);
#endif /* BIMODAL */
		break;
	case 56:
#if (BIMODAL_SUPPORT != 0)
		sscanf(value, "%lf", &lm_percent);
#endif /* BIMODAL */
		break;
	case 58:
		sscanf(value, "%lf", &trigger_rate);
		break;
	case 59:
		param = strtok(value, sep);
		trigger_max = trigger_min = 0;
		if (param)
			trigger_max = trigger_min = atoi(param);
		param = strtok(NULL, sep);
		if (param)
			trigger_min = atoi(param);
		if (trigger_min>trigger_max){ // Just in case
			long aux;
			aux=trigger_min;
			trigger_min=trigger_max;
			trigger_max=aux;
		}
		break;
	case 60:
		sscanf(value, "%d", &faults);
		break;
#if (EXECUTION_DRIVEN != 0)
	case 100:
		sscanf(value, "%ld", &fsin_cycle_relation);
		break;
	case 101:
		sscanf(value, "%ld", &simics_cycle_relation);
		break;
	case 103:
		sscanf(value, "%ld", &serv_addr);
		break;
	case 104:
		sscanf(value, "%ld", &num_periodos_espera);
		break;
#else
	case 100:
	case 101:
	case 103:
	case 104:
		break;
#endif

	default:
		panic("Should not be here in get_option");
	}
}

/**
* Verifies the simulation configuration.
*
* Looks for illegal values of the variables or not allowed combinations.
*/
void verify_conf(void) {
	char mon[128];

	if(pkt_len < 1 || phit_size < 1)
		panic("verify_conf: Illegal packet length");
	if( !(bub_adap[1]<buffer_cap && bub_x<buffer_cap && bub_y<buffer_cap && bub_z<buffer_cap))
		panic("Illegal bubble size");
	tr_ql = buffer_cap * pkt_len + 1;
	inj_ql = binj_cap * pkt_len + 1;

	if (topo == ICUBE && nways!=2){
		printf("WARNING: only bidirectional icubes implemented!!!\n");
		nways=2;
	}

	if (topo > CUBE && nways!=1){
		printf("WARNING: nways has no sense in indirect topologies !!!\n");
		nways=1;
	}

	if ((req_mode == DOUBLE_OBLIVIOUS_REQ && (nchan % ndim))) {
		printf("WARNING: Bubble double oblivious only for a number of VC multiple of ndim\n");
		printf("         Setting nchan to %d\n", ndim);
		nchan = ndim;
	}

	if ((req_mode == DOUBLE_ADAPTIVE_REQ && (nchan % ndim))) {
		printf("WARNING: CURRENTLY double adaptive only for a number of VC multiple of ndim\n");
		printf("         Setting nchan to %d\n", ndim);
		nchan = ndim;
	}

	if (ndim == 1)
		nodes_y = nodes_z = 1;           // Just in case...
	else if (ndim == 2)
		nodes_z = 1;

	// Direct topologies are mesh, torus, ttorus and midimew.
	if (topo < DIRECT) {
		NUMNODES = (nodes_x*nodes_y*nodes_z);
		nprocs = NUMNODES;
		radix=ndim*nways;
		if (placement==SHUFFLE_PLACE){
			printf("WARNING: Shuffle placement only for topologies with more than 1 node attached\n");
			printf("         to switching elements. Setting placement to column placement.\n");
			placement=COLUMN_PLACE;
		}
	}
	else if (topo==FATTREE){
		nprocs = (long)pow(stDown, nstages);
		NUMNODES = ((long)pow(stDown, nstages-1) * nstages) + nprocs;
		req_mode = TREE_REQ;
		vc_management = TREE_MANAGEMENT;
		if (inj_mode!=SHORTEST_INJ){
			printf("WARNING: Using pre-routing in injection with no cube topology\n");
			printf("         Setting imode to shortest.\n");
			inj_mode=SHORTEST_INJ;
		}
	}
	// slimmed (aka slender) trees
	else if (topo==SLIMTREE){
		long st, aux;
		if ((stDown % stUp)!=0) // it also panics if stUp > stDown.
			panic("Error in slimtree definition: stDown not divisible by stUp");
		nprocs = (long)pow((stDown/stUp),nstages)*stUp*stUp;
		aux = stUp;
		NUMNODES= stUp + nprocs;
		for (st=1; st<nstages; st++){
			aux=aux*stDown/stUp; //nodes in each stage.
			NUMNODES+=aux;
		}
		req_mode = TREE_REQ;
		vc_management = TREE_MANAGEMENT;
		if (inj_mode!=SHORTEST_INJ){
			printf("WARNING: Using pre-routing in injection with no cube topology.\n");
			printf("         Setting imode to shortest.\n");
			inj_mode=SHORTEST_INJ;
		}
	}
	// thin tree
	else if (topo==THINTREE){
		long st, aux;
		aux = (long)pow(stDown, nstages-1);
		nprocs = aux * stDown; // stDown ^ nstages
		NUMNODES= aux + nprocs;
		for (st=2; st<=nstages; st++){
			aux=stUp * (aux / stDown); //nodes in each stage. :: stUp^N-1*(stDown/stUp)^(N-st)
			NUMNODES+=aux;
		}
		req_mode = TREE_REQ;
		vc_management = TREE_MANAGEMENT;
		if (inj_mode!=SHORTEST_INJ){
			printf("WARNING: Using pre-routing in injection with no cube topology.\n");
			printf("         Setting imode to shortest.\n");
			inj_mode=SHORTEST_INJ;
		}
	}
	else if (topo==ICUBE){
		nprocs = nodes_per_switch * (nodes_x*nodes_y*nodes_z);
		NUMNODES = (nodes_per_switch+1) * (nodes_x*nodes_y*nodes_z);
		radix= nodes_per_switch + (ndim*2*links_per_direction);
		req_mode = ICUBE_REQ;
		vc_management = ICUBE_MANAGEMENT;
		if (inj_mode!=SHORTEST_INJ){
			printf("WARNING: Using pre-routing in injection with no cube topology.\n");
			printf("         Setting imode to shortest.\n");
			inj_mode=SHORTEST_INJ;
		}
	}

	if ((inj_mode >= DOR_INJ) && (ninj < (ndim*nways))) {
		ninj = ndim*nways;
		printf("WARNING: Injection with prerouting; setting the number of injectors to %ld\n", ninj);
	}

	n_ports = radix*nchan + ninj + 1;

	if (topo == MIDIMEW)
	{
		if (ndim != 1)
			panic("Midimew networks allow only one parameter");
		else
			ndim=2;
	}

	if (topo == TWISTED){
		if (sk_xy >= nodes_y)
			panic("dtt_neighbor: Skew too large");
		if (sk_xz >= nodes_z)
			panic("dtt_neighbor: Skew too large");
		if (sk_yx >= nodes_x)
			panic("dtt_neighbor: Skew too large");
		if (sk_yz >= nodes_z)
			panic("dtt_neighbor: Skew too large");
		if (sk_zx >= nodes_x)
			panic("dtt_neighbor: Skew too large");
		if (sk_zy >= nodes_y)
			panic("dtt_neighbor: Skew too large");

		// Only skews of two dimensions over the third one are permited this must be checked
		// in the initialization.
		if (sk_xy !=0){
			if ((sk_xz !=0)||(sk_yx !=0)||(sk_yz !=0)||(sk_zx !=0))
				panic("routing_r dtt_rr: Skew combination not allowed");
		}
		else if (sk_xz !=0){
			if ((sk_xy !=0)||(sk_yx !=0)||(sk_zx !=0)||(sk_zy !=0))
				panic("routing_r dtt_rr: Skew combination not allowed");
		}
		else if (sk_yx !=0){
			if ((sk_yz !=0)||(sk_xy !=0)||(sk_xz !=0)||(sk_zy !=0))
				panic("routing_r dtt_rr: Skew combination not allowed");
		}
		else if (sk_yz !=0){
			if ((sk_yx !=0)||(sk_xy !=0)||(sk_zx !=0)||(sk_zy !=0))
				panic("routing_r dtt_rr: Skew combination not allowed");
		}
		else if (sk_zx !=0){
			if ((sk_zy !=0)||(sk_xy !=0)||(sk_xz !=0)||(sk_yz !=0))
				panic("routing_r dtt_rr: Skew combination not allowed");
		}
		else if (sk_zy !=0){
			if ((sk_zx !=0)||(sk_xz !=0)||(sk_yx !=0)||(sk_yz !=0))
				panic("routing_r dtt_rr: Skew combination not allowed");
		}
		else{ // If all skews are 0 then we are in a torus
			printf("WARNING: All skews of this twisted torus are 0. Switching to torus topology\n");
			topo=TORUS;
		}
	}
	// other topologies must be defined

	if (vc_management == BUBBLE_MANAGEMENT &&
		req_mode != BUBBLE_ADAPTIVE_RANDOM_REQ &&
		req_mode != BUBBLE_ADAPTIVE_SHORTEST_REQ &&
		req_mode != BUBBLE_ADAPTIVE_SMART_REQ &&
		req_mode != BUBBLE_OBLIVIOUS_REQ
#if (BIMODAL_SUPPORT != 0)
		&& req_mode != BIMODAL_REQ
#endif /* BIMODAL */
		)
			panic("Incorrect request mode specification for bubble VC management");
	else if (vc_management == DOUBLE_MANAGEMENT &&
		req_mode != DOUBLE_OBLIVIOUS_REQ &&
		req_mode != DOUBLE_ADAPTIVE_REQ &&
		req_mode != HEXA_OBLIVIOUS_REQ &&
		req_mode != HEXA_ADAPTIVE_REQ)
			panic("Incorrect request mode specification for double VC management");
	else if(vc_management == DALLY_MANAGEMENT &&
		req_mode != DALLY_TRC_REQ &&
		req_mode != DALLY_BASIC_REQ &&
		req_mode != DALLY_IMPROVED_REQ &&
		req_mode != DALLY_ADAPTIVE_REQ)
			panic("Incorrect request mode specification for Dally VC management");

	if (pattern == HOTREGION && nprocs < 8)
		panic("Hotregion traffic pattern require more than 8 nodes");
	if (pattern == TRACE){
		drop_packets=FALSE;	// If some packet are dropped the simulation will never end.
		extract=0;		// Same as previous.
		samples=1;		// For final summary
		shotmode=FALSE;
		load=bg_load;   // generation rate of the background traffic.
		if (trcfile==NULL)
			panic("Trace file not defined");
		printf("WARNING: Trace driven mode: Disabling packet dropping & extracting\n");
#if (BIMODAL_SUPPORT != 0)
		printf("         Setting off bimodal injection\n");
		msglength=1;	 // Bimodal injection not allowed while using traces.
		lm_percent=0.0;
#endif /* BIMODAL */
		printf("         Setting samples to 1\n");

		// Let's check the placement parameters
		if (trace_nodes==0){
			trace_nodes=nprocs;
			trace_instances=1;
		}
		if (trace_instances==0)
			trace_instances=1;

		if (placement==QUADRANT_PLACE)
			if (topo<DIRECT){
				trace_instances=(long)pow(2, ndim);
				printf("WARNING: quadrant placement with a %d-D topology trace_instances is now %d\n",ndim, trace_instances);
			}
			else
				panic("quadrant placement only for k-ary n-cube topologies");
		if (placement==DIAGONAL_PLACE)
			if (ndim!=2 || trace_instances!=1 || topo>DIRECT)
				panic("diagonal placement only for 2d cube topologies and 1 instance");
		if (trace_nodes*trace_instances>nprocs)
			panic("Too much nodes and/or instances for this trace");
		if (trigger_rate>=0.0) {
		    printf("WARNING: Deactivating causal synthetic traffic: trigger_rate=0\n");
		    trigger_rate=0.0;
		}
	}

	if (req_mode > TWO_OR_MORE_REQUIRED && nchan < 2)
		panic("Two or more virtual channels required");

	if (req_mode > THREE_OR_MORE_REQUIRED && nchan < 3)
		panic("Three or more virtual channels required");

	if (req_mode > SIX_REQUIRED && nchan !=6)
		panic("Six virtual channels are required");

	if (vc_management == DALLY_MANAGEMENT && bub_x != 0) {
		printf("WARNING: Dally routing; setting bubble to 0\n");
		bub_x = bub_y = bub_z  = 0;
	}

#if (BIMODAL_SUPPORT != 0)
	if (req_mode != BIMODAL_REQ){
		msglength=1;
		lm_percent=0.0;
	}
#endif /* BIMODAL */

	if (shotmode) {
		if (shotsize == 0)
			shotsize = (nprocs-1);
	}

	if (max_conv_time==0)
		max_conv_time = (CLOCK_TYPE) 1000000L; // Should have converged in less than a million cycles.

#if (BIMODAL_SUPPORT != 0)
	lm_prob = lm_percent/(msglength-(lm_percent*(msglength-1)));
	aload = (long) (load * RAND_MAX * (msglength * (1-lm_prob) + lm_prob) / (pkt_len * msglength));
	lm_load = aload * lm_prob ;
#else
	// is the same as above when msglength=1 & lm_percent=0 (bimodal: off)
	aload = (long) ( (load/pkt_len) * RAND_MAX);
#endif /* BIMODAL */

	if (aload<0) //Because an overflow
		aload = RAND_MAX;

	trigger = trigger_rate * RAND_MAX;
	trigger_dif = 1 + trigger_max - trigger_min;

	switch (topo) {
		case MESH:
			if (!update_period)
				update_period=1;
			else
				update_period = update_period*(nodes_x+nodes_y+nodes_z-3);
			break;
		case TORUS:
			if (!update_period)
				update_period=1;
			else
				update_period = update_period*((nodes_x/2)+(nodes_y/2)+(nodes_z/2));
			break;
		default:
			if (!update_period)
				update_period=1;
			printf("WARNING: Using global congestion control with neither TORUS nor MESH topology, \n         Using value "PRINT_CLOCK"\n", update_period);
			break;
	}

	if (monitored>NUMNODES)
		monitored=0;

	if (strcmp(file, "")==0){
#ifdef WIN32
		sprintf(file, "fsin.out" , GetCurrentProcessId() );
#else
#include <unistd.h>
		sprintf(file, "fsin.%ld.out", getpid());
#endif
	   printf("WARNING: Output files undefined! using %s for default\n", file);
	}

#if (EXECUTION_DRIVEN != 0)
	num_executions = 0;
	sprintf(mon, "%s.%d.mon", file, num_executions);
#else
	sprintf(mon, "%s.mon", file);
#endif

	if((fp = fopen(mon, "w")) == NULL)
		panic("cannot create monitorized output file");

	if (timeout_upper_limit>0 && timeout_lower_limit<0)
		timeout_lower_limit = (timeout_upper_limit * 3) / 4;

	if (timeout_upper_limit<1)
		bub_adap[0]=bub_adap[1];

	if (topo==ICUBE && routing!=ADAPTIVE_ROUTING && nchan>1){
		printf("WARNING: multiple VC not allowed with static routing in icubes. Setting nchan to 1\n");
		nchan=1;
	}

	if (topo!=ICUBE && (req_mode==ICUBE_REQ || vc_management==ICUBE_MANAGEMENT))
		panic("Only for indirect cubes");
	if ((topo!=FATTREE && topo!=THINTREE && topo!=SLIMTREE) &&
		(req_mode==TREE_REQ || vc_management==TREE_MANAGEMENT))
		panic("Only for trees");
	if (topo>CUBE)
		nodes_x=stDown;		// This way the results will be printed in columns that are the number of nodes attached to each switch.
}

/**
* The default configuration is set here.
*/
void set_default_conf (void) {
	r_seed = 17;

	pkt_len = 16;
	phit_size = 4;
	pattern = UNIFORM;
	load = 1.0;
	bg_load = 0.0;
	drop_packets = FALSE;
	extract = FALSE;

	topo = TORUS;

	nodes_x = 4;
	nodes_y = 4;
	nodes_z = 1;

	nways = 2;
	nchan = 1;
	ninj = 1;
	buffer_cap = 4;
	binj_cap = 8;
	tr_ql = buffer_cap * pkt_len + 1;
	inj_ql = binj_cap * pkt_len + 1;

	sk_xy = sk_xz = sk_yx = sk_yz = sk_zx = sk_zy = 0;

	vc_management = BUBBLE_MANAGEMENT;
	routing = DIMENSION_ORDER_ROUTING;
	req_mode = BUBBLE_OBLIVIOUS_REQ;
	bub_x = bub_y = bub_z = 2;
	arb_mode = ROUNDROBIN_ARB;
	intransit_pr = 0.0;
	cons_mode = MULTIPLE_CONS;
	inj_mode = SHORTEST_INJ;
	parallel_injection = TRUE;
	plevel = 0;
	pinterval = (CLOCK_TYPE) 1000L;
	pheaders = 2047;
	bheaders = 8191;
	monitored = 1;
	bub_adap[0] = 0;
	bub_adap[1] = 0;
	shotmode = FALSE;
	shotsize = 1;
	global_cc = 100.0;
	update_period = (CLOCK_TYPE) 0L;

#if (BIMODAL_SUPPORT != 0)
	msglength=8;
	lm_percent=0.1;
#endif /* BIMODAL */

	warm_up_period= (CLOCK_TYPE) 25000L;

	conv_period= (CLOCK_TYPE) 1000L;
	threshold=0.05;
	max_conv_time= (CLOCK_TYPE) 25000L;
	trace_nodes=0;
	trace_instances=0;

	samples=25;
	batch_time=(CLOCK_TYPE) 2000L;
	min_batch_size=0;

	timeout_upper_limit= (CLOCK_TYPE) -1L;
	timeout_lower_limit= (CLOCK_TYPE) -1L;

	ndim=2;
	radix=4;
	nstages=0;
	placement=CONSECUTIVE_PLACE;
	shift=0;
	trace_instances=1;
	trace_nodes=0;

	faults=0;
	sprintf(trcfile,"/dev/null");

	trigger_rate=0.0;
	trigger_max=1;
	trigger_min=1;

#if (EXECUTION_DRIVEN != 0)
	fsin_cycle_relation = 10;
	simics_cycle_relation = 100;
	serv_addr = 8082;
	num_periodos_espera = 0;
#endif
}
