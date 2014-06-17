/**
* @file
* @brief	Printing Functions for FSIN.

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

#include <time.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "globals.h"

//	pheaders: 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512 + 1024

/**
* An array containing all the headers for runtime printing.
* 
* @see pheaders
*/
char * pheader[] =
{
	"      clock",
	", injload",   /* Injected load */
	", accload",   /* Consumed load */
	", avgdel",    /* Average latency */
	", stddel",    /* Standard deviation of latency */
	", maxdel",    /* Maximum latency */
	", avginjdel", /* Average injection latency */
	", stdinjdel", /* Standard deviation oj injection latency */
	", maxinjdel", /* Maximum injection latency */
	", inj. limit, cur. occup.",
};

/**
* An array containing all batch results headers.
* 
* @see bheaders
*/
char *bheader[] =
{
	",  BatchTime",
	", AvDistance",
	",    InjLoad",
	",    AccLoad",
	", PacketSent",
	", PacketRcvd",
	", PacketDrop",
	",   AvgDelay",
	", StDevDelay",
	",   MaxDelay",
	",  InjAvgDel",
	", InjStDvDel",
	",  InjMaxDel",
};

/**
* Print headers at simulation start.
* 
* @see pheaders.
*/
void print_headers(void) {
	unsigned long i;

	for(i = 0; i < 10; ++i)
		if(pheaders & (1 << i))
			printf("%s", pheader[i]);
	printf("\n\n");
	if (pheaders & 1024)
	    fprintf(fp, "Monitoring node %d\n\n", monitored);
}

/**
* Print partial stats at runtime.
*
* Besides write the evolution of the monitored node in the '.mon' file.
* 
* @see pheaders.
* @see file.
*/
void print_partials(void) {
	double inj_load;
	double cons_load;
	double rcvd;
	channel e;
	CLOCK_TYPE copyclock;

	copyclock = sim_clock - last_reset_time;
	rcvd=rcvd_count-last_rcvd_count;

	if(pheaders & 1)
		printf("%11"PRINT_CLOCK, sim_clock);
	if(pheaders & 2){
		inj_load = (double) (sent_phit_count) / (1.0 * nprocs * copyclock);
		printf(", %7.5lf", inj_load);
	}
	if(pheaders & 4){
		cons_load = (double) (rcvd_phit_count) / (1.0 * nprocs * copyclock);
		printf(", %7.5lf", cons_load);
	}
	if(pheaders & 8)
		printf(", %6.2lf", acum_delay / rcvd);
	if(pheaders & 16)
		printf(", %6.2lf", sqrt(fabs((acum_sq_delay-(acum_delay*acum_delay)/rcvd)/(rcvd-1))));
	if(pheaders & 32)
		printf(", %6ld", max_delay);
	if(pheaders & 64)
		printf(", %9.5lf", acum_inj_delay/sent_count);
	if(pheaders & 128)
		printf(", %9.5lf", sqrt(fabs((acum_sq_inj_delay-(acum_inj_delay*acum_inj_delay)/sent_count)/(sent_count-1))));
	if(pheaders & 256)
		printf(", %9ld", max_inj_delay);
	if(pheaders & 512)
		printf(", %10.0lf, %11.0lf", congestion_limit, global_q_u_current);
	printf("\n");
	if(pheaders & 1024){
		fprintf(fp, "%"PRINT_CLOCK, sim_clock);
		for (e=0; e<n_ports; e++){
			fprintf(fp, ",%4ld", queue_len(&(network[monitored].p[e].q)));
		}
		fprintf(fp, "\n");
	}
}

/**
* Prints the final summary & results.
*
* Prints the final summary that contains all the simulation parameters.
* Prints the results of each batch
*
* @see bheaders.
* @see plevel
* @see file.
*/
void print_results(time_t start_time, time_t end_time) {
	long i, j, c;
	channel e;
	unsigned long cn_size = 1024;
	char computer_name[1024];
	char *topo_s, *vc_s, *routing_s, *pattern_s, *ctype_s, *reqtype_s, *arbtype_s, *inj_s, *placement_s;
	CLOCK_TYPE copyclock;

	char map[256], hst[256];

	double res[13]={0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	double res_sq[13]={0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

	literal_name(pattern_l, &pattern_s, pattern);
	literal_name(topology_l, &topo_s, topo);
	literal_name(vc_l, &vc_s, vc_management);
	literal_name(routing_l, &routing_s, routing);
	literal_name(rmode_l, &reqtype_s, req_mode);
	literal_name(atype_l, &arbtype_s, arb_mode);
	literal_name(ctype_l, &ctype_s, cons_mode);
	literal_name(injmode_l, &inj_s, inj_mode);
	literal_name(placement_l, &placement_s, placement);

	samples = reseted ;

#ifdef WIN32
	GetComputerName(computer_name, &cn_size);
#else
	gethostname(computer_name, cn_size);
#endif

/* INPUT SECTION */
	// Execution's parameters
	printf("\n===============================================================================================================================================================\n\n");

	printf("Program Started at:               %s", asctime(localtime(&start_time)));
	printf("Ended at:                         %s", asctime(localtime(&end_time)));
	printf("On machine:                       %s\n", computer_name);
	printf("Simulation clock:                 %"PRINT_CLOCK"\n", sim_clock);
	printf("Total execution time:             ");
	i=end_time-start_time;
	if (i>86400){
		printf("%d days, ",i/86400);
		i=i%86400;
	}
	if (i>3600){
		printf("%d hours, ",i/3600);
		i=i%3600;
	}
	if (i>60){
		printf("%d mins, ",i/60);
		i=i%60;
	}
	printf("%d secs\n",i);
	printf("Cycles per Second:                %f\n\n", 1.0*sim_clock/(end_time-start_time));

	// Random seed
	printf("Random seed:                      %ld\n\n", r_seed);

	// Network & router's details
	printf("Topology, uni(1)/bidir(2):        %s(%ld)", topo_s, nways);

	if (topo < DIRECT){
		if (ndim == 1)
			printf(" 1-D: %ld = %ld nodes\n", nodes_x, NUMNODES);
		else if (ndim == 2)
			printf(" 2-D: %ldx%ld = %ld nodes\n", nodes_x, nodes_y, NUMNODES);
		else if (ndim == 3)
			printf(" 3-D: %ldx%ldx%ld = %ld nodes\n", nodes_x, nodes_y, nodes_z, NUMNODES);

		if (topo == TWISTED){
			printf("        with skews:                     ");
			if (sk_xy!=0)
				printf("xy=%d, ", sk_xy);
			if (sk_xz!=0)
				printf("xz=%d, ", sk_xz);
			if (sk_yx!=0)
				printf("yx=%d, ", sk_yx);
			if (sk_yz!=0)
				printf("yz=%d, ", sk_yz);
			if (sk_zx!=0)
				printf("zx=%d, ", sk_zx);
			if (sk_zy!=0)
				printf("zy=%d", sk_zy);
			printf("\n");
		}
		
		if (topo == CIRC_PK){
			printf("                                  a=%d, k=%d, adjacency: %d, %d\n\n",a,k,s1,s2);
		}
		
		if (topo == CIRCULANT){
			printf("                                  with adjacency: %d, %d\n\n",1,step);
		}
	}
	else if (topo == FATTREE){
		printf(" %ld-ary %ld-tree\n",radix/2,nstages);
		printf("nodes, switches(radix):           %-10ld %ld (%d)\n", nprocs, NUMNODES - nprocs, radix);
	}
	else if (topo == SLIMTREE){
		printf(" %d:%d-ary %ld-slimtree\n",stDown,stUp,nstages);
		printf("nodes, switches(radix):           %-10ld %-10ld (%d)\n", nprocs, NUMNODES - nprocs, radix);
	}
	else if (topo == THINTREE){
		printf(" %d:%d-ary %ld-thin-tree\n",stDown,stUp,nstages);
		printf("nodes, switches(radix):           %-10ld %-10ld (%d)\n", nprocs, NUMNODES - nprocs, radix);
	}
	else if (topo == ICUBE){
		printf("%dx%dx%d\n",nodes_x,nodes_y,nodes_z);
		printf("nodes per switch, parallel links: %-10ld %-10ld\n", nodes_per_switch, links_per_direction);
		printf("nodes, switches(radix):           %-10ld %-10ld (%d)\n", nprocs, NUMNODES - nprocs, radix);
	}
	printf("Random link failures:             %ld\n\n",faults);
	
	printf("Operation modes Inj-Req-Arb-Con:  %s %s %s %s\n", inj_s, reqtype_s, arbtype_s, ctype_s);
	printf("Traf./Inj. queue len (pkt/ph):    %ld/%ld, %ld/%ld, %ld injectors\n", (tr_ql-1)/pkt_len, tr_ql-1, (inj_ql-1)/pkt_len, inj_ql-1, ninj);
	printf("VC management:                    %s, %ld VCs; ", vc_s, nchan);
	if (vc_management==BUBBLE_MANAGEMENT || vc_management==DOUBLE_MANAGEMENT)
		printf("bubbles = %ld %ld %ld %ld pk.\n", bub_adap[1], bub_x, bub_y, bub_z);

	if (topo<DIRECT)
		printf("Oblivious routing for ESCAPE VCs: %s\n", routing_s);
	else
		printf("\nRouting in multistage network:    %s\n", routing_s);

	printf("Parallel injection:               ");
	if (parallel_injection)
		printf("YES\n");
	else
		printf("NO\n");

	printf("Dropping/Extracting packets:      ");
	if (drop_packets)
		printf("YES      ");
	else
		printf("NO       ");
	if (extract)
		printf("YES\n");
	else
		printf("NO\n");

	// Congestion control details.
	printf("\nGlobal congestion control:        gcc=%3.0f%%, update_period=%"PRINT_CLOCK"\n", global_cc, update_period);
	printf("In-transit priority, bub. to ad:  ");
	if (bub_adap[1])
		printf("%lf, YES (%ld)\n\n", intransit_pr, bub_adap[1]);
	else
		printf("%lf, NO\n\n", intransit_pr);

#if (EXECUTION_DRIVEN != 0)
	printf("Traffic pattern:                  EXECUTION DRIVEN, packets of %ld phits\n", pkt_len);
#else
	if (pattern == TRACE) {// Traffic details.
		printf("Traffic from trace:               %d instances of %s, packets of %ld phits\n", trace_instances, trcfile, pkt_len);
		if (placement==SHIFT_PLACE)
			printf("Placement:                        %s %d\n", placement_s, shift);
		else if (placement==FILE_PLACE)
			printf("Placement:                        %s %s\n", placement_s, placefile);
		else
			printf("Placement:                        %s\n", placement_s);

	    printf("Background uniform traffic at:    %1.5f\n", load);
	}
	else
	    if (shotmode){
			printf("Traffic pattern:                  %s, packets of %ld phits\n", pattern_s, pkt_len);
			printf("Shot mode (numshots x shotsize):  YES (%ld x %ld)\n", samples, shotsize);
			}
		else{
		    printf("Traffic pattern:                  %s at load %1.5f, packets of %ld phits\n", pattern_s, load, pkt_len);
			printf("Trigger rate, packets triggered:  %1.5f %1.5f", trigger_rate, trigger_min);
			if (trigger_min!=trigger_max)
			    printf("..%1.5f", trigger_max);
			printf("\nWarm Up Period Prov., Used:       %"PRINT_CLOCK " + %"PRINT_CLOCK", %"PRINT_CLOCK"\n", warm_up_period, max_conv_time, warmed_up);
			printf("Conv. sampling period, threshold: %"PRINT_CLOCK", %lf\n", conv_period, threshold);
			printf("Sample count, size, min pkts:     %ld x %"PRINT_CLOCK", %ld\n", samples, batch_time, min_batch_size);
		}

#endif

#if (BIMODAL_SUPPORT != 0)
	// Simulation's results
	// Bimodal injection
	if (msglength>1 && lm_prob>0){
		msg_sent_count[LONG_MSG] += msg_sent_count[LONG_LAST_MSG];
		msg_rcvd_count[LONG_MSG] += msg_rcvd_count[LONG_LAST_MSG];

		msg_acum_sq_delay[LONG_MSG] += msg_acum_sq_delay[LONG_LAST_MSG];
		msg_acum_delay[LONG_MSG] += msg_acum_delay[LONG_LAST_MSG];
		if (msg_max_delay[LONG_LAST_MSG] > msg_max_delay[LONG_MSG])
			msg_max_delay[LONG_MSG] = msg_max_delay[LONG_LAST_MSG];

		msg_acum_sq_inj_delay[LONG_MSG] += msg_acum_sq_inj_delay[LONG_LAST_MSG];
		msg_acum_inj_delay[LONG_MSG] += msg_acum_inj_delay[LONG_LAST_MSG];
		if (msg_max_inj_delay[LONG_LAST_MSG] > msg_max_inj_delay[LONG_MSG])
			msg_max_inj_delay[LONG_MSG] = msg_max_inj_delay[LONG_LAST_MSG];

		printf("\nBimodal Injection statistics:\n");
		printf("Short messages  Size   Inj.   Rcv.:   %4d %10.0f %10.0f\n", 1,
				msg_sent_count[SHORT_MSG], msg_rcvd_count[SHORT_MSG]);
		printf("Long messages   Size   Inj.   Rcv.:   %4d %10.0f %10.0f\n", msglength,
				msg_sent_count[LONG_LAST_MSG], msg_rcvd_count[LONG_LAST_MSG]);
		printf("Long pkt prop.  Prov.  Occur.:             %10.5f %10.5f\n",
				lm_percent,	msg_sent_count[LONG_MSG] / (msg_sent_count[LONG_MSG] + msg_sent_count[SHORT_MSG]));
		printf("Long msg prop.  Calc.  Occur.:             %10.5f %10.5f\n\n",
				lm_prob,msg_sent_count[LONG_LAST_MSG] / (msg_sent_count[LONG_LAST_MSG] + msg_sent_count[SHORT_MSG]));

		printf("Short msg pkts  Inj.   Rcv.:              %10.0f %10.0f\n",
				msg_sent_count[SHORT_MSG], msg_rcvd_count[SHORT_MSG]);
		printf("+>Delay  Avg.   StDev. Max.:   %10.5f %10.5f %10ld\n",
				msg_acum_delay[SHORT_MSG] / msg_rcvd_count[SHORT_MSG],
				sqrt(fabs((msg_acum_sq_delay[SHORT_MSG] - (msg_acum_delay[SHORT_MSG] * msg_acum_delay[SHORT_MSG]) / msg_rcvd_count[SHORT_MSG]) / (msg_rcvd_count[SHORT_MSG] - 1))),
				msg_max_delay[SHORT_MSG]);
		printf("+>InjDel Avg.   StDev. Max.:   %10.5f %10.5f %10ld\n\n",
				msg_acum_inj_delay[SHORT_MSG] / msg_rcvd_count[SHORT_MSG],
				sqrt(fabs((msg_acum_sq_inj_delay[SHORT_MSG] - (msg_acum_inj_delay[SHORT_MSG] * msg_acum_inj_delay[SHORT_MSG]) / msg_rcvd_count[SHORT_MSG]) / (msg_rcvd_count[SHORT_MSG]-1))),
				msg_max_inj_delay[SHORT_MSG]);

		printf("Long msg pkts   Inj.   Rcv.:              %10.0f %10.0f\n", msg_sent_count[LONG_MSG], msg_rcvd_count[LONG_MSG]);
		printf("+>Delay  Avg.   StDev. Max.:   %10.5f %10.5f %10ld\n",
				msg_acum_delay[LONG_MSG] / msg_rcvd_count[LONG_MSG],
				sqrt(fabs((msg_acum_sq_delay[LONG_MSG] - (msg_acum_delay[LONG_MSG] * msg_acum_delay[LONG_MSG]) / msg_rcvd_count[LONG_MSG]) / (msg_rcvd_count[LONG_MSG] - 1))),
				msg_max_delay[LONG_MSG]);
		printf("+>InjDel Avg.   StDev. Max.:   %10.5f %10.5f %10ld\n\n",
				msg_acum_inj_delay[LONG_MSG] / msg_rcvd_count[LONG_MSG],
				sqrt(fabs((msg_acum_sq_inj_delay[LONG_MSG] - (msg_acum_inj_delay[LONG_MSG] * msg_acum_inj_delay[LONG_MSG]) / msg_rcvd_count[LONG_MSG]) / (msg_rcvd_count[LONG_MSG] - 1))),
				msg_max_inj_delay[LONG_MSG]);
	}
#endif /* BIMODAL */

	printf("\n===============================================================================================================================================================\n");

	// Batch results
	printf("\n  #");
	for ( i=0; i<13; i++)
		if (bheaders & (1 << i))
			printf("%s",bheader[i]);

	copyclock= (CLOCK_TYPE) 0L;
	for (i=0; i<reseted; i++) {
		// Batch number
		printf("\n%3ld", i);

		if (bheaders & 1){
			printf(", %10"PRINT_CLOCK, batch[i].clock);
			res[0]  += batch[i].clock;
			res_sq[0]  += (batch[i].clock * (double) batch[i].clock);
		}
		if (bheaders & 2){
			printf(", %10.5f", batch[i].avDist);
			res[1]  += batch[i].avDist;
			res_sq[1]  += (batch[i].avDist * batch[i].avDist);
		}
		if (bheaders & 4){
			printf(", %10.5f", batch[i].inj_load);
			res[2]  += batch[i].inj_load;
			res_sq[2]  += (batch[i].inj_load * batch[i].inj_load);
		}
		if (bheaders & 8){
			printf(", %10.5f", batch[i].acc_load);
			res[3]  += batch[i].acc_load;
			res_sq[3]  += (batch[i].acc_load * batch[i].acc_load);
		}
		if (bheaders & 16){
			printf(", %10.0f", batch[i].sent_count);
			res[4]  += batch[i].sent_count;
			res_sq[4]  += (batch[i].sent_count * (double) batch[i].sent_count);
		}
		if (bheaders & 32){
			printf(", %10.0f", batch[i].rcvd_count);
			res[5]  += batch[i].rcvd_count;
			res_sq[5]  += (batch[i].rcvd_count * (double) batch[i].rcvd_count);
		}
		if (bheaders & 64){
			printf(", %10.0f", batch[i].dropped_count);
			res[6]  += batch[i].dropped_count;
			res_sq[6]  += (batch[i].dropped_count * (double) batch[i].dropped_count);
		}
		if (bheaders & 128){
			printf(", %10.2f", batch[i].avg_delay);
			res[7]  += batch[i].avg_delay;
			res_sq[7]  += (batch[i].avg_delay * batch[i].avg_delay);
		}
		if (bheaders & 256){
			printf(", %10.2f", batch[i].stDev_delay);
			res[8]  += batch[i].stDev_delay;
			res_sq[8]  += (batch[i].stDev_delay * batch[i].stDev_delay);
		}
		if (bheaders & 512){
			printf(", %10ld", batch[i].max_delay);
			res[9]  += batch[i].max_delay;
			res_sq[9]  += (batch[i].max_delay * (double) batch[i].max_delay);
		}
		if (bheaders & 1024){
			printf(", %10.2f", batch[i].avg_inj_delay);
			res[10] += batch[i].avg_inj_delay;
			res_sq[10] += (batch[i].avg_inj_delay * batch[i].avg_inj_delay);
		}
		if (bheaders & 2048) {
			printf(", %10.2f", batch[i].stDev_inj_delay);
			res[11] += batch[i].stDev_inj_delay;
			res_sq[11] += (batch[i].stDev_inj_delay * batch[i].stDev_inj_delay);
		}
		if (bheaders & 4096) {
			printf(", %10ld", batch[i].max_inj_delay);
			res[12] += batch[i].max_inj_delay;
			res_sq[12] += (batch[i].max_inj_delay * (double) batch[i].max_inj_delay);
		}
		// copyclock: cycles taken for the sampling period.
		copyclock += batch[i].clock;
	}
#if (EXECUTION_DRIVEN != 0)
		copyclock = sim_clock - last_reset_time;
#endif

	if(pattern!=TRACE){ // In trace-driven there's only 1 sample: no AVG nor STD.
		printf("\n\nAVG");
		if (bheaders & 1)
			printf(", %10.2f", (res[0]/samples));
		if (bheaders & 2)
			printf(", %10.5f", (res[1]/samples));
		if (bheaders & 4)
			printf(", %10.5f", (res[2]/samples));
		if (bheaders & 8)
			printf(", %10.5f", (res[3]/samples));
		if (bheaders & 16)
			printf(", %10.2f", (res[4]/samples));
		if (bheaders & 32)
			printf(", %10.2f", (res[5]/samples));
		if (bheaders & 64)
			printf(", %10.2f", (res[6]/samples));
		if (bheaders & 128)
			printf(", %10.2f", (res[7]/samples));
		if (bheaders & 256)
			printf(", %10.2f", (res[8]/samples));
		if (bheaders & 512)
			printf(", %10.2f", (res[9]/samples));
		if (bheaders & 1024)
			printf(", %10.2f", (res[10]/samples));
		if (bheaders & 2048)
			printf(", %10.2f", (res[11]/samples));
		if (bheaders & 4096)
			printf(", %10.2f", (res[12]/samples));
		printf("\nSTD");

		if (bheaders & 1)
			printf(", %10.2f", sqrt(fabs((res_sq[0] - (res[0]*res[0]) / samples) / (samples-1)) ));
		if (bheaders & 2)
			printf(", %10.5f", sqrt(fabs((res_sq[1] - (res[1]*res[1]) / samples) / (samples-1)) ));
		if (bheaders & 4)
			printf(", %10.5f", sqrt(fabs((res_sq[2] - (res[2]*res[2]) / samples) / (samples-1)) ));
		if (bheaders & 8)
			printf(", %10.5f", sqrt(fabs((res_sq[3] - (res[3]*res[3]) / samples) / (samples-1)) ));
		if (bheaders & 16)
			printf(", %10.2f", sqrt(fabs((res_sq[4] - (res[4]*res[4]) / samples) / (samples-1)) ));
		if (bheaders & 32)
			printf(", %10.2f", sqrt(fabs((res_sq[5] - (res[5]*res[5]) / samples) / (samples-1)) ));
		if (bheaders & 64)
			printf(", %10.2f", sqrt(fabs((res_sq[6] - (res[6]*res[6]) / samples) / (samples-1)) ));
		if (bheaders & 128)
			printf(", %10.2f", sqrt(fabs((res_sq[7] - (res[7]*res[7]) / samples) / (samples-1))	));
		if (bheaders & 256)
			printf(", %10.2f", sqrt(fabs((res_sq[8] - (res[8]*res[8]) / samples) / (samples-1)) ));
		if (bheaders & 512)
			printf(", %10.2f", sqrt(fabs((res_sq[9] - (res[9]*res[9]) / samples) / (samples-1)) ));
		if (bheaders & 1024)
			printf(", %10.2f", sqrt(fabs((res_sq[10] - (res[10]*res[10]) / samples) / (samples-1)) ));
		if (bheaders & 2048)
			printf(", %10.2f", sqrt(fabs((res_sq[11] - (res[11]*res[11]) / samples) / (samples-1)) ));
		if (bheaders & 4096)
			printf(", %10.2f", sqrt(fabs((res_sq[12] - (res[12]*res[12]) / samples) / (samples-1)) ));
		printf("\n");
	}

	if(plevel & 64) {
		fprintf(fp, "\n\nSource ports:        ");
		for(e=0;e<p_inj_first;e++)
			fprintf(fp, ", %5ld ", source_ports[e]);
		fprintf(fp, "\n\nDestination ports:   ");
		for(e=0;e<p_inj_first;e++)
			fprintf(fp, ", %5ld ", dest_ports[e]);
		fprintf(fp, "\n\nChannel utilization: ");
		for(e=0;e<p_inj_first;e++)
			fprintf(fp, ", %5lf ", 1.0*port_utilization[e]/copyclock);
// End of monitored node information
		fclose(fp);
	}

	if (plevel & 1 || plevel & 2){
// Maps for all the network
#if (EXECUTION_DRIVEN != 0)
		sprintf(map, "%s.%d.map", file, num_executions);
#else
		sprintf(map, "%s.map", file);
#endif

		if((fp = fopen(map, "w")) == NULL)
			printf("WARNING: cannot create network mapping output file");
		else{
			if(plevel & 1) {
				fprintf(fp, "MAPS OF SOURCES AND DESTINATIONS\n");
				fprintf(fp, "AT INJECTION\n");
				for(i=0;i<nprocs;i++)
					for (j=0;j<nprocs;j++)
						if (j)
							fprintf(fp, ", %5ld", sources[i][j]);
						else
							fprintf(fp, "\n%5ld", sources[i][j]);

				fprintf(fp, "\n\nAT CONSUMPTION\n");
				for(i=0;i<nprocs;i++)
					for (j=0;j<nprocs;j++)
						if (j)
							fprintf(fp, ", %5ld", destinations[i][j]);
						else
							fprintf(fp, "\n%5ld", destinations[i][j]);
			}
			if(plevel & 2){
				fprintf(fp, "\n\nMAPS OF CHANNEL UTILIZATION\n\n");
				for(e=0;e<p_inj_first;e++){
					fprintf(fp, "MAP OF CHANNEL %ld:", e);
					if (NUMNODES==nprocs)
						i=0;
					else
						i=nprocs;	// Do not print the NICs.
						for(;i<NUMNODES;i++)
							if (i % nodes_x)
								fprintf(fp, ", %4lf", 1.0*network[i].p[e].utilization/copyclock);
							else
								fprintf(fp, "\n%4lf", 1.0*network[i].p[e].utilization/copyclock);

					fprintf(fp, "\n\n");
				}
			}
		fclose(fp);
		}
	}

	if(plevel & 4 || plevel & 8){
// End of network mapping information

// Histogram for all nodes
#if (EXECUTION_DRIVEN != 0)
		sprintf(hst, "%s.%d.hst", file, num_executions);
#else
		sprintf(hst, "%s.hst", file);
#endif
		if((fp = fopen(hst, "w")) == NULL)
			printf("WARNING: cannot create histograms output file");
		else{
			if (plevel & 4){
				fprintf(fp, "HISTOGRAM OF DISTANCE\n\n Distance, Injection, Consumption\n");
				for (i=1;i<max_dst;i++)
					fprintf(fp," %8ld, %8ld, %8ld\n", i, inj_dst[i], con_dst[i]);
				fprintf(fp,"\n\n");
			}
			if (plevel & 8){
				fprintf(fp, "HISTOGRAM OF PORT UTILIZATION\n\n");
				fprintf(fp, "   Node, Port,  Empty");
				for (c=1; c<buffer_cap; c++)
				fprintf(fp, ", %6ld", c);
				fprintf(fp, ",   Full\n");

				if (NUMNODES==nprocs)
					i=0;
				else
					i=nprocs;	// In the NICS there are no traffic
				for(;i<NUMNODES;i++) // Node
					for (e=0; e<=p_inj_last; e++){ // port
						fprintf(fp, " %8ld, %4ld", i, e);
						for (c=0; c<=buffer_cap; c++) // occupancy
							fprintf(fp, ", %"PRINT_CLOCK, network[i].p[e].histo[c]);
						fprintf(fp, "\n");
					}
				fprintf(fp, "\n");
			}
			// End of node histogram information
			fclose(fp);
		}
	}

#if (EXECUTION_DRIVEN != 0)
	sprintf(map, "%s.%d.mon", file, ++num_executions);
	if((fp = fopen(map, "w")) == NULL)
		printf("WARNING: cannot create monitored output file");
#endif
}
