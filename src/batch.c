/**
* @file
* @brief	Tools & running modes for sampling (batchs & shots).
*
*@author Javier Navaridas

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

#include "globals.h"

/** Current Consumed Load. */
double cons_load;

/** Current Average Latency. */
double latency;

/** Previous Consumed Load (during convergency ensurement). */
double prev_cons_load;

/** Previous Average Latency (during convergency ensurement). */
double prev_latency;

/** Number of convergences occurred in a row (during convergency ensurement).
 * When it is three we move to the result capturing phase.
 */
static long convergence;

/**
* Print the results of a batch in an Human Readable Style (not very dense).
*
* Print all batch's stats in HR style. The stats shown are:
* cycles taken, average distance, injected load, Accepted load,
* packet sent count, received packet count, dropped packet count,
* average delay, delay standard deviation, maximun delay,
* average injection delay, injection delay standard deviation,
* maximun injection delay.
*
* Not actually in use, but may be useful.
*
* @param b a pointer to the batch to print
*/
void print_batch_results_vast(batch_t *b){
	printf("\n ---------------------------------------------\n");

	if (reseted==0)
		printf("                Warmed Up !!!!!!\n");
	else
		printf("                Sample #%2ld !!!!!!\n", reseted);

	printf(" ---------------------------------------------\n\n");

	printf("Sample time:                      %10"PRINT_CLOCK"\n", b->clock);
	printf("AvDistance:                       %10.5lf\n", b->avDist);
	printf("Packets  Inj.   Rcv.   Drop.:     %10lf %10lf %10lf\n", b->sent_count, b->rcvd_count, b->dropped_count);
	printf("Phit     Inj.   Rcv.   Drop.:     %10lf %10lf %10lf\n", b->sent_phit_count, b->rcvd_phit_count, b->dropped_phit_count);
	printf("Load     Prov.  Inj.   Acc.:      %10.5lf %10.5lf %10.5lf\n", load, b->inj_load, b->acc_load);
	printf("Delay    Avg.   StDev. Max.:      %10.5lf %10.5lf %10ld\n", b->avg_delay, b->stDev_delay, b->max_delay);
	printf("InjDelay Avg.   StDev. Max.:      %10.5lf %10.5lf %10ld\n\n",b->avg_inj_delay, b->stDev_inj_delay, b->max_inj_delay);
}

/**
* Print the results of a batch in a Comma Separated Value format (dense).
*
* Print all batch's stats in CSV format.  The stats shown are:
* cycles taken, average distance, injected load, Accepted load,
* packet sent count, received packet count, dropped packet count,
* average delay, delay standard deviation, maximun delay,
* average injection delay, injection delay standard deviation,
* maximun injection delay.
* @param b a pointer to the batch to print
*/
void print_batch_results(batch_t *b){
	printf("Batch %ld, %"PRINT_CLOCK", %f, %f, %f, %f, %f, %f, %f, %f, %ld, %f, %f, %ld\n\n",
			reseted, b->clock,
			b->avDist,
			b->inj_load, b->acc_load,
			b->sent_count, b->rcvd_count, b->dropped_count,
			b->avg_delay, b->stDev_delay, b->max_delay,
			b->avg_inj_delay, b->stDev_inj_delay, b->max_inj_delay);
}

/**
* Save the current batch's stats.
*
* Save the current stats in the #reseted position of the batch array. The stats saved are:
* cycles taken, average distance, injected load, Accepted load,
* packet sent count, received packet count, dropped packet count,
* average delay, delay standard deviation, maximun delay,
* average injection delay, injection delay standard deviation,
* maximun injection delay.
*/
void save_batch_results(){
	CLOCK_TYPE copyclock;
	double rcvd;
	copyclock = sim_clock - last_reset_time; // time taken for this batch.

	batch[reseted].clock = copyclock;
	batch[reseted].sent_count = sent_count;
	batch[reseted].rcvd_count = rcvd = rcvd_count-last_rcvd_count;
	batch[reseted].avDist = (1.0 * acum_hops) / rcvd;
	batch[reseted].dropped_count = dropped_count + transit_dropped_count - last_tran_drop_count;
	batch[reseted].sent_phit_count = sent_phit_count;
	batch[reseted].rcvd_phit_count = rcvd_phit_count;
	batch[reseted].dropped_phit_count = dropped_phit_count;
	batch[reseted].inj_load = (double) (sent_phit_count) / (1.0 * nprocs * copyclock);
	batch[reseted].acc_load = (double) (rcvd_phit_count) / (1.0 * nprocs * copyclock);
	batch[reseted].avg_delay = acum_delay/rcvd;
	batch[reseted].stDev_delay = sqrt(fabs((acum_sq_delay-(acum_delay*acum_delay)/rcvd)/(rcvd-1)));
	batch[reseted].max_delay = max_delay;
	batch[reseted].avg_inj_delay = acum_inj_delay/rcvd;
	batch[reseted].stDev_inj_delay = sqrt(fabs((acum_sq_inj_delay-(acum_inj_delay*acum_inj_delay)/rcvd)/(rcvd-1)));
	batch[reseted].max_inj_delay = max_inj_delay;
}

/**
* Run the warm-up period.
*
* Run the simulation for #warm_up_period cycles.
*
* @see run_network_batch()
* @see warm_up_period
*/
void warm_up(void){
	while (sim_clock < warm_up_period && !interrupted && !aborted){
		data_movement(B_TRUE);
		sim_clock++;

		if ((pheaders > 0) && (sim_clock % pinterval == 0))
			print_partials();

		if (sim_clock % update_period == 0){
			global_q_u = global_q_u_current;
			global_q_u_current = injected_count - rcvd_count - transit_dropped_count;
		}
	}
}

/**
* Does the system converge?.
*
* Calculates the % of difference beetwen current & previous load & latency.
*
* @return TRUE if current and previous figures and are within the given threshold or FALSE in other case.
* @see convergency()
* @see run_network_batch()
*/
bool_t system_converges(void){
	CLOCK_TYPE copyclock = sim_clock - last_reset_time;
	double dif_load, dif_latency;

	cons_load = (double) (rcvd_phit_count)/(1.0 * nprocs * copyclock);
	latency = acum_delay/(rcvd_count - last_rcvd_count);
	dif_load = fabs((cons_load - prev_cons_load)/cons_load);
	dif_latency = fabs((latency - prev_latency)/latency);

	return (bool_t)(dif_load < threshold && dif_latency < threshold);
}

/**
* The simulation continues to assure convergency.
*
* Run the simulation and each #conv_period cycles estimates the convergency of the system.
* This phase ends when they are 3 converged samples in a row or when it spends more
* than #max_conv_time cycles without reach the stationary state.
* A message stating the reason of leaving this phase is printed.
*
* @see system_converges()
* @see run_network_batch()
*/
void convergency(void){
	long converged=B_FALSE;
	go_on=B_TRUE;
	while (go_on && !interrupted  && !aborted){
		data_movement(B_TRUE);
		sim_clock++;
		if ((pheaders > 0) && (sim_clock % pinterval == 0))
			print_partials();
		if (sim_clock % conv_period == 0 ){
			if ( system_converges() )
				convergence++;
			else
				convergence=0;

			if (convergence==3){
				go_on=B_FALSE;
				converged=B_TRUE;
			}
			prev_cons_load = cons_load;
			prev_latency = latency;
			reset_stats();
		}
		if (sim_clock % update_period == 0){
			global_q_u = global_q_u_current;
			global_q_u_current = injected_count - rcvd_count - transit_dropped_count;
		}
		if (sim_clock - warm_up_period >= max_conv_time)
			go_on=B_FALSE;
	}

	// Adjust for equal sample adquiring
	while (sim_clock % batch_time != 0 && !interrupted  && !aborted){
		data_movement(B_TRUE);
		sim_clock++;
		if ((pheaders > 0) && (sim_clock % pinterval == 0))
			print_partials();

		if (sim_clock % update_period == 0){
			global_q_u = global_q_u_current;
			global_q_u_current = injected_count - rcvd_count - transit_dropped_count;
		}
	}
	warmed_up = sim_clock;
	reseted=-1;
	reset_stats();

    if (!interrupted && !aborted){
        if (converged){
            printf("\n ---------------------------------------------\n");
            printf("                Warmed Up !!!!!!               \n");
            printf(" ---------------------------------------------\n\n");
        }
        else{
            printf("\n**********************************************\n");
            printf("*        Convergency timeout reached.        *\n");
            printf("*         Continue without converge!         *\n");
            printf("**********************************************\n\n");
        }
    }
}

/**
* Stationary phase (steady-state) of the simulation where batch stats are taken.
*
* Continues the simulation for #samples batches of #batch_time cycles and at least
* #min_batch_size packets received. Now is the time for capturing simulation stats.
*
* @see run_network_batch()
*/
void stationary(void){
	go_on=B_TRUE;
	while (go_on && !interrupted  && !aborted){
		data_movement(B_TRUE);
		sim_clock++;
		if ((pheaders > 0) && (sim_clock % pinterval == 0))
			print_partials();

		if (sim_clock % update_period == 0){
			global_q_u = global_q_u_current;
			global_q_u_current = injected_count - rcvd_count - transit_dropped_count;
		}

		if (sim_clock % batch_time == 0 && (rcvd_count-last_rcvd_count) >= min_batch_size){
			save_batch_results();
			print_batch_results(&batch[reseted]);

			reset_stats();

			if (reseted == samples)
				go_on=B_FALSE;
		}
	}
}

/**
* Run the simulation taking stats for some batches.
*
* The simulation are split in three phases:
* Warm-up, Convergency assurement & Stationary state.
*
* @see warm_up()
* @see convergency()
* @see stationary()
*/
void run_network_batch(void){
	warm_up();
	convergency();
	stationary();
}

/**
* Run the simulation in shotmode.
*
* The simulation runs for #samples shots and takes stats for each one.
*/
void run_network_shotmode(void) {
	long iterations, i;

	reseted=-1;
	reset_stats();
	for (iterations=0; iterations<samples && !interrupted  && !aborted; iterations++) {
		printf("SHOT NUMBER %4ld started at time: %"PRINT_CLOCK"\n", iterations, sim_clock);
		printf("============================================\n");
		datagen_oneshot(B_TRUE);
		do {
			if ((sim_clock % update_period) == 0) {
				global_q_u = global_q_u_current;
				global_q_u_current = injected_count - rcvd_count - transit_dropped_count;
			}
			datagen_oneshot(B_FALSE);
			for (i=0; i<nprocs; i++) data_injection(i);
			data_movement(B_FALSE);
			sim_clock++;
			if ((pheaders > 0) && (sim_clock % pinterval == 0))
				print_partials();
		} while ((rcvd_count-last_rcvd_count) < total_shot_size && !interrupted  && !aborted);
		save_batch_results();
		print_batch_results(&batch[reseted]);
		reset_stats();
	}
}

