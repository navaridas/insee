/**
* @file
* @brief	Tools for collecting stats.

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

#include "globals.h"

/**
* Collects queue occupancy histogram stats.
* 
* @param i The node to collect from.
*/
void stats(long i) {
	long ql_p, ql_m;
	port_type e;
	port *pt;
	phit *p;

	for (e=0; e<n_ports; e++) {
		pt = &(network[i].p[e]);
		ql_p = queue_len(&(pt->q));
		ql_m = ql_p/pkt_len;
		if (ql_p) {
			p = head_queue(&(pt->q));
			if ((p->pclass != RR) && (p->pclass != RR_TAIL))
				ql_m++;
		}
		if (ql_m > buffer_cap + 1)
			panic("Too many packets");
		pt->histo[ql_m]++;
	}
}

/**
* Resets the stats of the simulation.
*
* Histograms, maps & long messages stats are only cleared when the convergency phase are finished.
*/
void reset_stats(void) {
	port_type e;
	long i, j;
#if (BIMODAL_SUPPORT != 0)
	message_l k;
#endif /* BIMODAL */

	sent_count = 0.0;
	last_rcvd_count = rcvd_count;
	last_tran_drop_count = transit_dropped_count;
	dropped_count=0;

	inj_phit_count = 0.0;
	sent_phit_count = 0.0;
	rcvd_phit_count = 0.0;
	dropped_phit_count = 0.0;

	acum_delay = 0.0;
	acum_inj_delay = 0.0;
	max_delay = 0;
	max_inj_delay = 0;
	acum_sq_delay = 0.0;
	acum_sq_inj_delay = 0.0;
	acum_hops = 0.0;

	if (reseted < 0){
		for (e=0; e<n_ports; e++){
			port_utilization[e]=(CLOCK_TYPE) 0L;
			source_ports[e]=0;
			dest_ports[e]=0;
		}

		for (i=0; i<NUMNODES; i++){
			for (e=0; e<p_inj_first; e++)
				network[i].p[e].utilization = (CLOCK_TYPE) 0L;

			if(plevel & 8)
				for (e=0; e<n_ports; e++)
					for (j=0; j<buffer_cap+1; j++)
						network[i].p[e].histo[j] = (CLOCK_TYPE) 0L;
		}
#if (BIMODAL_SUPPORT != 0)
		for (k=SHORT_MSG; k<=LONG_LAST_MSG; k++){
			msg_sent_count[k] = 0.0;
			msg_injected_count[k] = 0.0;
			msg_rcvd_count[k] = 0.0;
			msg_acum_delay[k] = 0.0;
			msg_acum_inj_delay[k] = 0.0;
			msg_max_delay[k] = 0;
			msg_max_inj_delay[k] = 0;
			msg_acum_sq_delay[k] = 0.0;
			msg_acum_sq_inj_delay[k] = 0.0;
		}
#endif /* BIMODAL */
	}
#if (EXECUTION_DRIVEN !=0)
	for (e=0; e<n_ports; e++){
		port_utilization[e]=(CLOCK_TYPE) 0L;
		source_ports[e]=0;
		dest_ports[e]=0;
	}

	if (plevel & 1)
		for (i=0; i<nprocs; i++)
			for (j=0; j<nprocs; j++){
				destinations[i][j]=0;
				sources[i][j]=0;
			}

	for (i=0; i<NUMNODES; i++){
		for (e=0; e<p_inj_first; e++)
			network[i].p[e].utilization = (CLOCK_TYPE) 0L;
		if(plevel & 8)
			for (e=0; e<n_ports; e++)
				for (j=0; j<buffer_cap+1; j++)
					network[i].p[e].histo[j] = (CLOCK_TYPE) 0L;
	}
#endif
	reseted++;
	last_reset_time = sim_clock;
}

/**
* Print partial results.
*/
void results_partial(void) {
	CLOCK_TYPE copyclock;

	copyclock = sim_clock - last_reset_time;

	if (pheaders > 0)
		print_partials();
}
