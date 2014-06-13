/**
* @file
* @brief	Main function of the FSIN simulator.
*/

/** @mainpage
FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011)

@author J. Miguel-Alonso, A. Gonzalez, F. J. Ridruejo, J. Navaridas.

@section intro Introduction

FSIN Functional Simulator for Interconnection Networks.

@section gnu License

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

@todo Stable Version !!!!
@todo Publish, Publish, Publish :)
*/

#ifndef _main
#define _main

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "globals.h"
#include "packet.h"

#endif

/* Global variables - parameters */

long  r_seed;		///< Random Seed

double load;		///< The provided injected load.
double trigger_rate;///< Probability to trigger new packets when a packet is received.
long trigger_max;	///< Maximum of packets triggered.
long trigger_min;	///< Minimum of packets triggered.
long trigger_dif;	///< Random interval == 1 + trigger_max - trigger_min.
long ndim;
long radix;			///< Radix of each router.
long nstages;		///< Number of stages for indirect topologies.
long stDown;		///< Number of links down in a slim/thin tree. It also refeers to the number of nodes attached to a switching element in fattree and indirect cube .
long stUp;			///< Number of links up in a slim tree.
long NUMNODES;		///< Total number of nodes.
long n_ports;		///< Total number of ports in each router.
long nprocs;		///< Total number nodes that are able to inject.

long nodes_x;		///< Nodes in dimension X.
long nodes_y;		///< Nodes in dimension Y.
long nodes_z;		///< Nodes in dimension Z.
long nchan;			///< Number of Virtual Channels.
long nways;			///< Number of ways. (unidir. / bidir.)
long ninj;			///< Number of injectors in each router.
long pkt_len;		///< Length of FSIN packets in phits.
long phit_size;		///< Size, in bytes, of a phit. Used in application-driven simulation for splitting mpi messages into packets.
long buffer_cap;	///< Transit buffer capacity (in packets).
long binj_cap;		///< Injection buffer capacity (in packets).

long pnodes_x;		///< Nodes in dimension X of the submesh for icube placement.
long pnodes_y;		///< Nodes in dimension Y of the submesh for icube placement.
long pnodes_z;		///< Nodes in dimension Z of the submesh for icube placement.

long faults;		///< Number of broken links.

/**
* Transit queue length (in phits).
*
* Equal to #buffer_cap multiplied by #pkt_len.
* 
* @see buffer_cap.
* @see pkt_len.
*/
long tr_ql;

/**
* Injection queue length (in phits).
*
* Equel to #binj_cap multiplied by #pkt_len.
* 
* @see binj_cap.
* @see pkt_len.
*/
long inj_ql;

/**
* The traffic pattern Id.
* 
* @see traffic_pattern_t.
* @see pattern_l.
*/
traffic_pattern_t pattern;

/**
* The topology Id.
* 
* @see topo_t.
* @see topology_l.
*/
topo_t topo;

/**
* Id of the Virtual Channel management mechanism.
* 
* @see vc_management_t
* @see vc_l
*/
vc_management_t vc_management;

/**
* Id of the routing used in DOR (Dimension or Direction) and in Multistage networks.
* 
* @see routing_t
* @see routing_l
*/
routing_t routing;

/**
* Id of the request port mechanism.
* 
* @see req_mode_t
* @see r_mode_l
*/
req_mode_t req_mode;

/**
* Id of the router arbitration mechanism.
* 
* @see arb_mode_t
* @see atype_l
*/
arb_mode_t arb_mode;

/**
* Id of the consumption mode.
* 
* @see cons_mode_t
* @see ctype_l
*/
cons_mode_t cons_mode;

/**
* Id of the injection mode.
* 
* @see inj_mode_t
* @see injmode_l
*/
inj_mode_t inj_mode;

/**
* Id of the placement strategy.
* 
* @see placement_t
* @see placement_l
*/
placement_t placement;

long shift;				///< Number of places for shift placement.
long trace_nodes;		///< Number of tasks in the trace
long trace_instances;	///< Number of instances of the trace to simulate
char placefile[128];

bool_t parallel_injection;			///< Allows/Disallows the parallel injection (inject some packets in the same cycle & router).

long bub_adap[2],					///< Bubble to adaptive channels.
	 bub_x,							///< Bubble to X Dimension Escape Channels. Usually equal to Y,Z.
	 bub_y,							///< Bubble to Y Dimension Escape Channels. Usually equal to X,Z.
	 bub_z;							///< Bubble to Z Dimension Escape Channels. Usually equal to X,Y.
double intransit_pr;				///< Priority given to in-transit traffic.
double global_cc;					///< Global congestion control. Percent of the system recurses used.
long congestion_limit;	///< congestion limit calculated from global_cc.
CLOCK_TYPE update_period;					///< Period taken to calculate the global ocupation.
CLOCK_TYPE timeout_upper_limit;	///< Timeout limit to enter in congested mode.
CLOCK_TYPE timeout_lower_limit;	///< Timeout limit to leave congested mode.

long monitored;						///< Monitored node.

/**
* Print level - bitmap.
*  0 = Just final summary
*  1 = Source/destination Map
*  2 = port utilization Map
*  4 = Distance Histogram
*  8 = Histogram of port utilization
* 16 = Packet-level traces
* 32 = Phit-level traces
* 64 = Monitored node
*/
long plevel;
CLOCK_TYPE pinterval; ///< Interval to print information about the system state.

/**
* Partial headers. A Bitmap to know what system stats are shown.
* 1     + 2        + 4          + 8        + 16        + 32        + 64        + 128     + 256       + 512        + 1024
* clock   inj_load   cons_load    av_delay   delay_dev   max_delay   inj_delay   i_d_dev   i_dev_max   netw. ocup   queue_ocup
*/
long pheaders;

/**
* Batch headers. A Bitmap to know what batch stats are printed in final summary.
* 1        + 2         + 4      + 8      + 16        + 32        + 64        + 128     + 256       + 512     + 1024     + 2048      + 4096
* BatchTime  AvDistance  InjLoad  AccLoad  PacketSent  PacketRcvd  PacketDrop  AvgDelay  StDevDelay  MaxDelay  InjAvgDel  InjStDvDel  InjMaxDel
*/
long bheaders;

bool_t extract;				///< Extract packets from head of the injection queue when they cannot be injected.
bool_t drop_packets;		///< Drop packets that have no room in the injection queue.

bool_t shotmode;			///< Are the system running in shotmode.
long shotsize;				///< Size of each shot (in packets per node)
long total_shot_size;		///< Number of packets in each shot.

#if (BIMODAL_SUPPORT != 0)
long msglength;					///< The length of the long messages (in packets) for bimodal traffic.
double lm_prob,					///< Probability of inject a long message in bimodal traffic(in packets).
	lm_percent;					///< Probability of inject a long message in bimodal traffic(in messages).

double msg_sent_count[3];		///< Number of Message sent of each type. (bimodal stats)
double msg_injected_count[3];	///< Number of Message injected of each type. (bimodal stats)
double msg_rcvd_count[3];		///< Number of Message received of each type. (bimodal stats)

double msg_acum_delay[3],		///< Accumulative delay of each message type. (bimodal stats)
	msg_acum_inj_delay[3];		///< Accumulative injection delay of each message type. (bimodal stats)
double msg_acum_sq_delay[3],	///< Accumulative square delay of each message type. (bimodal stats)
	msg_acum_sq_inj_delay[3];	///< Accumulative square injection delay of each message type. (bimodal stats)
long msg_max_delay[3],			///< Maximum delay of each message type. (bimodal stats)
	msg_max_inj_delay[3];		///< Maximum injection delay of each message type. (bimodal stats)
#endif /* BIMODAL */

FILE *fp; ///< A pointer to a file. Used for several purposes.

/**
* The output file name.
*
* It will be appended one extension for each out:
* .mon for monitored node.
* .map for channel mapping.
* .hst for queue occupation histograms.
*/
char file[128];

#if (EXECUTION_DRIVEN != 0)
long num_executions;	///< Number of executions, appended to the output filename.
#endif

/**
* The name of a file containing a trace for trace driven.
*
* If the file is in .alog format & only will be considered message sending an reception (events -101 & -102).
* If the file is a dimemas trace only point to point operations and cpu intervals are considered.
* The FSIN trace format is also allowed.
*/
char trcfile[128];

long samples;			///< Number of samples (batchs or shots) to take from the current Simulation.
CLOCK_TYPE batch_time;		///< Sampling period.
long min_batch_size;	///< Minimum number of reception in a batch to save stats.

double threshold;		///< Threshold to accept convergency.
CLOCK_TYPE warm_up_period,	///< Number of 'oblivious' cycles until start convergency asurement simulation.
	warmed_up;			///< The cycle in wich warming are really finished.
CLOCK_TYPE conv_period;		///< Convergency estimation sampling period.
CLOCK_TYPE max_conv_time;		///< Maximum time for Convergency estimation.

/* Global variables - other */

router *network;		///< An array of routers containing the system.

CLOCK_TYPE sim_clock;			///< Simulation clock.

double sent_count = 0.0;			///< Number of packets sent. (stats)
double injected_count = 0.0;		///< Number of packets injected. (stats)
double rcvd_count = 0.0;			///< Number of packets received. (stats)
double last_rcvd_count = 0.0;		///< Number of packets received before the start of the present batch. (stats)
double dropped_count = 0.0;			///< Number of packets dropped. (stats)
double transit_dropped_count = 0.0;	///< Number of in-transit packets dropped. (stats)
double last_tran_drop_count = 0.0;	///< Number of in-transit packets dropped before the start of the present batch. (stats)
double inj_phit_count = 0.0;		///< Number of phits sent. (stats)
double sent_phit_count = 0.0;		///< Number of phits injected. (stats)
double rcvd_phit_count = 0.0;		///< Number of phits received. (stats)
double dropped_phit_count = 0.0;	///< Number of phits dropped. (stats)

double acum_delay = 0.0,		///< Accumulative delay. (stats)
	acum_inj_delay = 0.0;		///< Accumulative injection delay. (stats)
double acum_sq_delay = 0.0,		///< Accumulative square delay. (stats)
	acum_sq_inj_delay = 0.0;	///< Accumulative square injection delay. (stats)
long max_delay = 0,				///< Maximum delay. (stats)
	max_inj_delay = 0;			///< Maximum injection delay. (stats)
double acum_hops = 0.0;			///< Accumulative number of hops. (stats)

batch_t * batch;		///< Array to save all the batchs' stats.

/**
* The global queue occupation.
*
* This value is calculated each cycle.
* 
* @see update_period
* @see global_q_u_current
*/
double global_q_u_current = 0.0;

/**
* The global queue occupation.
*
* This is the value shown to the nodes and is updated every #update_period
* cycles with the value of #global_q_u_current.
* 
* @see update_period
* @see global_q_u_current
*/
double global_q_u = 0.0;

long aload,		///< Provided load multiplied by RAND_MAX used in injection.
	 lm_load,	///< Actual long message load multiplied by RAND_MAX used in bimodal injection.
	 trigger;	///< Provided trigger_rate multiplied by RAND_MAX used in reactive traffic.

long **destinations;	///< Matrix containing map of source/destination pairs at consumption (destination).
long **sources;			///< Matrix containing map of source/destination pairs at injection (source).
long * con_dst;			///< Histograms of distance at consumption (source).
long * inj_dst;			///< Histograms of distance at injection (source).
long max_dst;			///< Size of distance histograms.
long *source_ports;		///< Array that contains the source ports for the monitored node.
long *dest_ports;		///< Array that contains the destination ports for the monitored node.
CLOCK_TYPE *port_utilization;	///< Array that contains the port utilization of the monitored node.

dim * port_coord_dim;			///< An array containig the dimension for each port.
way * port_coord_way;			///< An array containig the direction (way) for each port.
channel * port_coord_channel;	///< An array containig the number of virtual channel for each port.

static time_t start_time,	///< Simulation start time / date.
				end_time;	///< Simulation finish time / date.

bool_t go_on = TRUE;		///< While this is TRUE the simulation will go on!

long reseted = 0;			///< Number of resets.
CLOCK_TYPE last_reset_time = 0;	///< Time in which the last reset has been performed.

double cons_load;	///< Consumed load of the current simulation.
double inj_load;	///< Injection load	of the current simulation.

/**
* 'Virtual' Function that runs the simulation.
* 
* @see init_functions
* @see run_network_shotmode
* @see run_network_trc
* @see run_network_batch
* @see run_network_exd
*/
void (* run_network)(void);

/**
* 'Virtual' Function that performs the selection of the injection ports.
* 
* @param i The node in wich the injection is performed.
* @param dest The destination of the packet.
* @return The injection port to insert the packet.
*
* @see init_functions
* @see select_input_port_shortest
* @see select_input_port_dor_only
* @see select_input_port_dor_shortest
* @see select_input_port_shortest_profitable
* @see select_input_port_lpath
*/
port_type (* select_input_port) (long i, long dest);

/**
* 'Virtual' Function that performs the search of a neighbor node. Only used by direct topologies.
* 
* @param ad A node address.
* @param wd A dimension (X,Y or Z).
* @param ww A way (UP or DOWN).
* @return The address of the neighbor in that direction and way.
*
* @see topo
* @see torus_neighbor
* @see midimew_neighbor
* @see torus_neighbor
* @see dtt_neighbor
*/
long (*neighbor)(long ad, dim wd, way ww);

/**
* 'Virtual' Function that calculates packet routing record.
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*
* @see init_functions
* @see topo
* @see mesh_rr
* @see midimew_rr
* @see torus_rr
* @see torus_rr_unidir
* @see dtt_rr
* @see dtt_rr_unidir
*/
routing_r (*calc_rr) (long source, long destination);

/**
* 'Virtual' Function that prepares the request of an output port.
* 
* @param i The node in which the request is performed.
* @param s_p The source (input) port which is requesting the output port.
*
* @see init_functions
* @see req_mode
* @see request_port_bubble_oblivious
* @see request_port_bubble_adaptive_random
* @see request_port_bubble_adaptive_shortest
* @see request_port_bubble_adaptive_smart
* @see request_port_double_oblivious
* @see request_port_double_adaptive
* @see request_port_hexa_oblivious
* @see request_port_hexa_adaptive
* @see request_port_dally_trc
* @see request_port_dally_basic
* @see request_port_dally_improved
* @see request_port_dally_adaptive
* @see request_port_bimodal_random
* @see request_port_tree
* @see void request_port_icube
* @see void request_port_icube_IB
*/
void (*request_port) (long i, port_type s_p);

/**
* 'Virtual' Function that arbitrates the selection of an input port for an output port.
* @param i The node in which the consumption is performed.
* @param d_p The destination port to arbitrate for.
* @param first The first input port that can use this output.
* @param last The next of the last input port that can use this input.
* @return The selected port or NULL_PORT if there is no one.
*
* @see init_functions
* @see arb_mode
* @see arbitrate
* @see arbitrate_select_round_robin
* @see arbitrate_select_fifo
* @see arbitrate_select_longest
* @see arbitrate_select_random
* @see arbitrate_select_age
*/
port_type (*arbitrate_select)(long i, port_type d_p, port_type first, port_type last);

/**
* 'Virtual' Function that arbritates the consumption of the arriven packets.
* @param i The node in which the consumption is performed.
*
* @see init_functions
* @see cons_mode
* @see arbitrate_cons_single
* @see arbitrate_cons_multiple
*/
void (*arbitrate_cons)(long i);

/**
* 'Virtual' Function that performs consumption.
* @param i The node in which the consumption is performed.
*
* @see init_functions
* @see cons_mode
* @see consume_single
* @see consume_multiple
*/
void (*consume)(long i);

/**
* 'Virtual' Function that routes a packet.
* 
* @param pkt The packet to route.
* @param d_d The destination dimension is returned here.
* @param d_w The destination way is returned here.
* @return TRUE if the phit has reached its final destination
*
* @see init_functions
* @see routing
* @see check_rr_dim_o_r
* @see check_rr_dir_o_r
* @see check_rr_fattree_static
* @see check_rr_fattree_adaptive
* @see check_rr_slimtree_static
* @see check_rr_slimtree_adaptive
* @see check_rr_thintree_static
* @see check_rr_thintree_adaptive
* @see check_rr_icube_static
* @see check_rr_icube_adaptive
* @see check_rr_icube_static_IB
*/
bool_t (*check_rr)(packet_t * pkt, dim *d_d, way *d_w);

/**
* 'Virtual' Function that performs the traffic movement in the network.
* 
* @param inject It is TRUE if the injection has to be performed.
*
* @see init_functions
* @see data_movement_direct
* @see data_movement_indirect
*/
void (*data_movement)(bool_t inject);

/**
* 'Virtual' Function that performs the arbitration of the output ports.
* 
* @param inject It is TRUE if the injection has to be performed.
*
* @see init_functions
* @see arbitrate_direct
* @see arbitrate_icube
* @see arbitrate_trees
*/
void (*arbitrate)(long i, port_type d_p);

/**
* Main function.
*
* Initializes the simulation & the network.
* Then runs the simulation & writes the results.
* 
* @param argc The number of parameters given in the command line.
* @param argv Array that constains all the parameters.
* @return The finalization code. Usually 0.
*/
int main(int argc, char *argv[]) {
	packet_t *pkt;
	long i,p;

	time(&start_time);
	get_conf((long)(argc - 1), argv + 1);
	sim_clock = 1L; // HAS TO BE ONE for arbitrate to work

	srand(r_seed);

	router_init();
	pkt_init();
	arbitrate_init();
	request_ports_init();

	init_functions();
	init_network();
	init_injection();

	if (pheaders > 0)
		print_headers();

	run_network();
	time(&end_time);
	print_results(start_time, end_time);
#ifdef WIN32
	system("PAUSE");
#endif
	return 0;
}
