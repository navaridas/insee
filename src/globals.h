/**
* @file
* @brief	Declaration of all global variables & functions.

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, J. Navaridas

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

#ifndef _globals
#define _globals

#include <math.h>
#include <time.h>
#include <limits.h>

#include "packet.h"
#include "constants.h"
#include "literal.h"
#include "misc.h"
#include "phit.h"
#include "queue.h"
#include "event.h"
#include "router.h"
#include "pkt_mem.h"
#include "batch.h"

#ifndef _main

extern long radix;
extern long ndim;
extern long nstages;
extern long stDown;
extern long stUp;
extern long NUMNODES;
extern long nprocs;
extern long n_ports;

extern long r_seed;
extern long nodes_x, nodes_y, nodes_z;
extern long binj_cap;
extern long ninj;
extern router * network;
extern long **destinations;
extern long **sources;
extern long * con_dst;
extern long * inj_dst;
extern long max_dst;
extern long *source_ports;
extern long *dest_ports;
extern CLOCK_TYPE *port_utilization;
extern double sent_count;
extern double injected_count;
extern double rcvd_count;
extern double last_rcvd_count;
extern double dropped_count;
extern double transit_dropped_count;
extern double last_tran_drop_count;

extern long pnodes_x;
extern long pnodes_y;
extern long pnodes_z;

extern long faults;

#if (BIMODAL_SUPPORT != 0)
 extern double msg_sent_count[3];
 extern double msg_injected_count[3];
 extern double msg_rcvd_count[3];

 extern double msg_acum_delay[3], msg_acum_inj_delay[3];
 extern long msg_max_delay[3], msg_max_inj_delay[3];
 extern double msg_acum_sq_delay[3], msg_acum_sq_inj_delay[3];

 extern long msglength;
 extern double lm_prob, lm_percent;
#endif /* BIMODAL */

extern CLOCK_TYPE sim_clock;
extern CLOCK_TYPE last_reset_time;
extern long bub_adap[2], bub_x, bub_y, bub_z;
extern topo_t topo;
extern long plevel;
extern long pheaders;
extern long bheaders;
extern CLOCK_TYPE pinterval;
extern long extract;
extern long monitored;
extern double intransit_pr;

extern double inj_phit_count;
extern double sent_phit_count;
extern double rcvd_phit_count;
extern double dropped_phit_count;

extern long (*neighbor)(long ad, dim wd, way ww);
extern routing_r (*calc_rr)(long source, long destination);
extern void (*request_port) (long i, port_type s_p);
extern void (*arbitrate_cons)(long i);
extern port_type (*arbitrate_select)(long i, port_type d_p, port_type first, port_type last);
extern void (*consume)(long i);
extern bool_t (*check_rr)(packet_t * pkt, dim *d_d, way *d_w);
extern port_type (* select_input_port) (long i, long dest);
extern void (*data_movement)(bool_t inject);
extern void (*arbitrate)(long i, port_type d_p);

extern double load, trigger_rate ;
extern long aload, lm_load, trigger;
extern long trigger_max, trigger_min, trigger_dif;
extern double global_q_u, global_q_u_current;
extern vc_management_t vc_management;
extern routing_t routing;
extern traffic_pattern_t pattern;
extern cons_mode_t cons_mode;
extern arb_mode_t arb_mode;
extern req_mode_t req_mode;
extern inj_mode_t inj_mode;

extern placement_t placement;
extern long shift;
extern long trace_nodes;
extern long trace_instances;
extern char placefile[128];

extern bool_t drop_packets;
extern bool_t parallel_injection;
extern bool_t shotmode;
extern long shotsize;
extern double global_cc;
extern long congestion_limit;
extern CLOCK_TYPE timeout_upper_limit;
extern CLOCK_TYPE timeout_lower_limit;

extern CLOCK_TYPE update_period;
extern long total_shot_size;

extern batch_t *batch;
extern long samples;
extern CLOCK_TYPE batch_time;
extern double threshold;
extern CLOCK_TYPE warm_up_period, warmed_up;
extern CLOCK_TYPE conv_period;
extern CLOCK_TYPE max_conv_time;
extern long min_batch_size;

extern double acum_delay, acum_inj_delay;
extern long max_delay, max_inj_delay;
extern double acum_sq_delay, acum_sq_inj_delay;
extern double acum_hops;

extern long nodes_per_switch;
extern long links_per_direction;

extern long pkt_len, phit_size, buffer_cap, tr_ql, inj_ql;

extern dim * port_coord_dim;
extern way * port_coord_way;
extern channel * port_coord_channel;

extern packet_t * pkt_space;
extern long pkt_max;

extern char trcfile[128];

extern bool_t go_on;
extern long reseted;
extern double threshold;

extern FILE *fp;
extern char file[128];

extern port_type last_port_arb_con;

extern void (* run_network)(void);

void run_network_shotmode(void);
void run_network_batch(void);

#endif // _main

/* In data_generation.c */
void init_injection (void);
void data_generation(long i);
void data_injection(long i);
void datagen_oneshot(bool_t reset);

void generate_pkt(long i);
port_type select_input_port_shortest(long i, long dest);
port_type select_input_port_dor_only(long i, long dest);
port_type select_input_port_dor_shortest(long i, long dest);
port_type select_input_port_shortest_profitable(long i, long dest);
port_type select_input_port_lpath(long i, long dest);

/* In arbitrate.c */
void reserve(long i, port_type d_p, port_type s_p);
void arbitrate_init(void);
void arbitrate_cons_single(long i);
void arbitrate_cons_multiple(long i);
void arbitrate_direct(long i, port_type d_p);
void arbitrate_icube(long i, port_type d_p);
void arbitrate_trees(long i, port_type d_p);
port_type arbitrate_select_fifo(long i, port_type d_p, port_type first, port_type last);
port_type arbitrate_select_longest(long i, port_type d_p, port_type first, port_type last);
port_type arbitrate_select_round_robin(long i, port_type d_p, port_type first, port_type last);
port_type arbitrate_select_random(long i, port_type d_p, port_type first, port_type last);
port_type arbitrate_select_age(long i, port_type d_p, port_type first, port_type last);

/* In request_ports.c */
void request_ports_init(void);
void request_port_bubble_oblivious(long i, port_type s_p);
void request_port_bubble_adaptive_random(long i, port_type s_p);
void request_port_bubble_adaptive_shortest(long i, port_type s_p);
void request_port_bubble_adaptive_smart(long i, port_type s_p);
void request_port_double_oblivious(long i, port_type s_p);
void request_port_double_adaptive(long i, port_type s_p);
void request_port_hexa_oblivious(long i, port_type s_p);
void request_port_hexa_adaptive(long i, port_type s_p);
void request_port_dally_trc(long i, port_type s_p);
void request_port_dally_basic(long i, port_type s_p);
void request_port_dally_improved(long i, port_type s_p);
void request_port_dally_adaptive(long i, port_type s_p);
void request_port_bimodal_random(long i, port_type s_p);

void request_port_tree(long i, port_type s_p);
void request_port_icube(long i, port_type s_p);
void request_port_icube_IB(long i, port_type s_p);

bool_t check_rr_dim_o_r(packet_t * pkt, dim *d_d, way *d_w);
bool_t check_rr_dir_o_r(packet_t * pkt, dim *d_d, way *d_w);
bool_t check_rr_fattree(packet_t * pkt, dim *d_d, way *d_w);
bool_t check_rr_fattree_adaptive(packet_t * pkt, dim *d, way *w);
bool_t check_rr_thintree_adaptive(packet_t * pkt, dim *d, way *w);
bool_t check_rr_slimtree_adaptive(packet_t * pkt, dim *d, way *w);
bool_t check_rr_icube_adaptive(packet_t * pkt, dim *d, way *w);
bool_t check_rr_icube_static_IB(packet_t * pkt, dim *d, way *w);
bool_t check_rr_fattree_static(packet_t * pkt, dim *d, way *w);
bool_t check_rr_thintree_static(packet_t * pkt, dim *d, way *w);
bool_t check_rr_slimtree_static(packet_t * pkt, dim *d, way *w);
bool_t check_rr_icube_static(packet_t * pkt, dim *d, way *w);


/* In perform_mov.c */
void phit_away(long, port_type, phit);
void consume_single(long i);
void consume_multiple(long i);
void advance(long n, long p);
void data_movement_direct(bool_t inject);
void data_movement_indirect(bool_t inject);

/* In init_functions.c */
void init_functions (void);

/* In stats.c */
void stats(long i);
void reset_stats(void);

/* In router.c */
extern long nchan;
extern long nways;
extern port_type p_con;
extern port_type p_drop;
extern port_type p_inj_first, p_inj_last;

void router_init(void);
void init_network(void);
void coords (long ad, long *cx, long *cy, long *cz);
void coords_icube (long ad, long *cx, long *cy, long *cz);

long torus_neighbor(long ad, dim wd, way ww);
long midimew_neighbor(long ad, dim wd, way ww);
long dtt_neighbor(long ad, dim wd, way ww);

routing_r torus_rr (long source, long destination);
routing_r torus_rr_unidir (long source, long destination);
routing_r mesh_rr (long source, long destination);
routing_r midimew_rr (long source, long destination);
routing_r dtt_rr (long source, long destination);
routing_r dtt_rr_unidir (long source, long destination);
routing_r icube_rr (long source, long destination);
routing_r icube_4mesh_rr (long source, long destination);
routing_r icube_1mesh_rr (long source, long destination);
routing_r fattree_rr_adapt (long source, long destination);
routing_r thintree_rr_adapt (long source, long destination);
routing_r slimtree_rr_adapt (long source, long destination);

void create_fattree();
void create_slimtree();
void create_thintree();
void create_icube();

/* In get_conf.c */
extern literal_t vc_l[];
extern literal_t routing_l[];
extern literal_t rmode_l[];
extern literal_t atype_l[];
extern literal_t ctype_l[];
extern literal_t pattern_l[];
extern literal_t topology_l[];
extern literal_t injmode_l[];
extern literal_t placement_l[];

void get_conf(long, char **);

/* In print_results.c */
void print_headers(void);
void print_partials(void);
void print_results(time_t, time_t);
void results_partial(void);

/* In batch.c */
void save_batch_results();
void print_batch_results(batch_t *b);
void print_batch_results_vast(batch_t *b);

/* In dtt.c */
extern long sk_xy, sk_xz, sk_yx, sk_yz, sk_zx, sk_zy; // Skews for twisted torus

/* In queue.c */
void init_queue (queue *q);

/* In pkt_mem.c */
void pkt_init();
void free_pkt(unsigned long n);
unsigned long get_pkt();

#if (TRACE_SUPPORT != 0)
 /* In trace.c */
 void read_trace();
 void run_network_trc();
 
/* In event.c */
 void init_event (event_q *q);
 void ins_event (event_q *q, event i);
 void do_event (event_q *q, event *i);
 event head_event (event_q *q);
 void rem_head_event (event_q *q);
 bool_t event_empty (event_q *q);
#endif /* TRACE common */

#if (TRACE_SUPPORT > 1)
 void init_occur (event_l **l);
 void ins_occur (event_l **l, event i);
 bool_t occurred (event_l **l, event i);
#endif /* TRACE multilist */

#if (TRACE_SUPPORT == 1)
 void init_occur (event_l *l);
 void ins_occur (event_l *l, event i);
 bool_t occurred (event_l *l, event i);
#endif /* TRACE single list */

#define packet_size_in_phits pkt_len
#if (EXECUTION_DRIVEN != 0)
  extern long fsin_cycle_relation;
  extern long simics_cycle_relation;
  extern long serv_addr;
  extern long num_periodos_espera;
  extern long num_executions;
  void init_exd(long, long, long, long, long);
  void run_network_exd(void);
#endif

#endif /* _globals */
