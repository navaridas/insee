/** 
* @file
* @brief	Definition of a batch.

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

#ifndef _batch
#define _batch

/**
* Structure to contain information (statistics) of a sample.
*
* Each samples is captured during a "batch" (when running using
* independent sources of synthetic traffic) or a "shot" (when 
* running in shot mode). The stored stats are:
* cycles taken, average distance, injected load, Accepted load, 
* packet sent count, received packet count, dropped packet count,
* average delay, delay standard deviation, maximun delay,
* average injection delay, injection delay standard deviation,
* maximun injection delay.
*/
typedef struct batch_t {
	CLOCK_TYPE clock;					///< Cycles taken for this batch.
	double avDist;				///< Average distance. 
	double sent_count;          ///< Packets sent.
	double rcvd_count;          ///< Packets received.
	double dropped_count;       ///< Packets dropped.
	double sent_phit_count;     ///< Phits sent.
	double rcvd_phit_count;     ///< Phits received.
	double dropped_phit_count;  ///< Phits dropped.
	double inj_load;		    ///< Averaged injected load.
	double acc_load;			///< Averaged accepted load.
	double avg_delay;		    ///< Averaged delay (includes injection delay plus transit delay).
	double stDev_delay;			///< Standard deviation of delay.
	long max_delay;				///< Maximum delay.
	double avg_inj_delay;		///< Averaged injection delay (time before entering in network).
	double stDev_inj_delay;		///< Standard deviation of injection delay.
	long max_inj_delay;			///< Maximum delay.
} batch_t;
#endif

